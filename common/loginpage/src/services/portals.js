import { createSignal, createEffect, on, onCleanup } from "solid-js";
import { onNativeMessage, onAuthChecked } from "@/sdk/events.js";
import { command } from "@/sdk/bridge.js";
import { getExternalClouds,
        sendProviderList,
        openPortal as sdkOpenPortal,
        logoutPortal as sdkLogoutPortal,
        keepPortalCookie} from "@/sdk/portals.js";

const STORAGE_KEY = "portals";

function skipProtocol(url) {
  return /^(?:https?:\/\/)?([^\s<>?&]+)/i.exec(url)?.[1] ?? url;
}

function readStorage() {
  try {
    const arr = JSON.parse(localStorage.getItem(STORAGE_KEY) || "[]");
    return [...arr].reverse();
  } catch {
    return [];
  }
}

function upsertStorage(info) {
  try {
    const arr = JSON.parse(localStorage.getItem(STORAGE_KEY) || "[]");
    const idx = arr.findIndex((p) => p.portal === info.portal);
    if (idx >= 0) arr.splice(idx, 1);
    arr.push(info);
    localStorage.setItem(STORAGE_KEY, JSON.stringify(arr));
  } catch {}
}

function deleteStorage(portalUrl) {
  try {
    const arr = JSON.parse(localStorage.getItem(STORAGE_KEY) || "[]");
    const idx = arr.findIndex((p) => p.portal === portalUrl);
    if (idx >= 0) {
      arr.splice(idx, 1);
      localStorage.setItem(STORAGE_KEY, JSON.stringify(arr));
    }
  } catch {}
}

const [providers, setProviders] = createSignal([]);
const [portals, setPortals] = createSignal(readStorage());

export { providers, portals };

// The desktop bridge answers `AscDesktopEditor.getClouds()` (asked from plugin
// frames, e.g. the AI chat) by evaluating `PortalsStore.portals()`
if (typeof window !== "undefined") {
  window.PortalsStore = {
    portals: () =>
      portals().map(({ portal, provider, user, email, logged }) => ({ portal, provider, user, email, logged })),
  };
}

export function initPortals() {
  const clouds = getExternalClouds();
  if (clouds.length) {
    setProviders(sortProviders(clouds));
    sendProviderList(clouds);
  }

  const _portals = portals();
  if (!!_portals && _portals.length)
    checkAuth(_portals);

  // Tell every frame (the AI chat plugin in particular) that the connected
  // clouds changed; the bridge broadcasts `window.onUpdateClouds()`.
  createEffect(on(portals, () => {
    window.AscDesktopEditor?.updateClouds?.();
  }, { defer: true }));

  const unsub = onNativeMessage((cmd, param) => {
    if (cmd === "portal:login") {
      let obj;
      try {
        obj = JSON.parse(param);
      } catch {
        return;
      }
      if (!obj?.displayName) return;

      const info = {
        portal: (obj.domain ?? "").replace(/\/$/, ""),
        provider: obj.provider || "onlyoffice",
        user: obj.displayName,
        email: obj.email || "",
      };

      const known = portals().find((p) => skipProtocol(p.portal) === skipProtocol(info.portal));
      const needCookie = info.provider !== "onlyoffice" && !(known?.logged && known.email === info.email);

      upsertStorage(info);
      setPortals(readStorage());

      if (needCookie) keepPortalCookie(info.portal);

      // `logged` is only ever set by an auth check; a portal signed in during
      // the session would otherwise stay unknown until the next start.
      checkAuth([info]);
    } else if (cmd === "portal:logout") {
      const name = skipProtocol(param);
      setPortals((prev) => prev.map((p) => (skipProtocol(p.portal) === name ? { ...p, logged: false } : p)));
    } else if (cmd === "portal:logout:cancel") {
      const name = skipProtocol(param);
      setPortals((prev) => prev.map((p) => (skipProtocol(p.portal) === name ? { ...p, removed: false } : p)));
    }
  });

  const unsubOnAuth = onAuthChecked((obj) => {
    if (!obj || typeof obj !== "object") return;
    setPortals((prev) =>
      prev.map((p) => {
        const key = skipProtocol(p.portal);
        if (!(key in obj)) return p;
        return { ...p, logged: obj[key] !== "" };
      }),
    );
  });
  onCleanup(()=>{unsub(); unsubOnAuth();});
}

export function openConnectedPortal(portal) {
  const cfg = providers().find((p) => p.provider === portal.provider);
  const entrypage = cfg?.startPage ?? "/";
  const target = cfg?.entryPage ? cfg.entryPage : portal.portal;
  sdkOpenPortal(target, portal.provider, entrypage);
}

export function logoutConnectedPortal(portal) {
  const cfg = providers().find((p) => p.provider === portal.provider);
  const extra = cfg?.entryPage
    ? cfg.extraLogout?.includes(cfg.entryPage)
      ? cfg.extraLogout
      : [...(cfg.extraLogout ?? []), cfg.entryPage]
    : undefined;
  sdkLogoutPortal(portal.portal, extra);
  setPortals((prev) => prev.map((p) => (p.portal === portal.portal ? { ...p, logged: false } : p)));
}

export function forgetConnectedPortal(portal) {
  const cfg = providers().find((p) => p.provider === portal.provider);
  const extra = cfg?.entryPage
    ? cfg.extraLogout?.includes(cfg.entryPage)
      ? cfg.extraLogout
      : [...(cfg.extraLogout ?? []), cfg.entryPage]
    : undefined;
  sdkLogoutPortal(portal.portal, extra);
  deleteStorage(portal.portal);
  setPortals((prev) => prev.filter((p) => p.portal !== portal.portal));
}

export function connectToProvider(provider, url) {
  command(
    "auth:outer",
    JSON.stringify({
      type: "outer",
      provider: provider?.provider ?? "",
      portal: url ?? "",
      entrypage: provider?.startPage ?? "/",
    }),
  );
}

function sortProviders(list = []) {
    return list.slice().sort((a, b) => {
        if (a.provider === 'onlyoffice' && b.provider !== 'onlyoffice') return -1;
        if (b.provider === 'onlyoffice' && a.provider !== 'onlyoffice') return 1;

        const aNoOrder = (a.order === undefined || a.order === null || a.order === '');
        const bNoOrder = (b.order === undefined || b.order === null || b.order === '');

        if (aNoOrder && !bNoOrder) return 1;
        if (!aNoOrder && bNoOrder) return -1;
        if (aNoOrder && bNoOrder) return 0;

        return Number(a.order) - Number(b.order);
    });
}

function checkAuth(portals) {
  let auth_arr = {};
  portals.forEach(p => {
    const displayName = p.portal.replace(/^https?:\/\//, "");
    auth_arr[displayName] = '';
  });

  const sdk = window.AscDesktopEditor;
  sdk.checkAuth(auth_arr);
}
