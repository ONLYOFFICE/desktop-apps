import { createSignal, onCleanup } from "solid-js";
import { onNativeMessage } from "@/sdk/events.js";
import { cmd } from "@/sdk/bridge.js";
import { getExternalClouds, sendProviderList, openPortal as sdkOpenPortal, logoutPortal as sdkLogoutPortal } from "@/sdk/portals.js";

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

export function initPortals() {
  const clouds = getExternalClouds();
  if (clouds.length) {
    setProviders(clouds);
    sendProviderList(clouds);
  }

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

      upsertStorage(info);
      setPortals(readStorage());
    } else if (cmd === "portal:logout") {
      const name = skipProtocol(param);
      setPortals((prev) => prev.map((p) => (skipProtocol(p.portal) === name ? { ...p, logged: false } : p)));
    } else if (cmd === "portal:logout:cancel") {
      const name = skipProtocol(param);
      setPortals((prev) => prev.map((p) => (skipProtocol(p.portal) === name ? { ...p, removed: false } : p)));
    }
  });

  onCleanup(unsub);
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
  cmd(
    "auth:outer",
    JSON.stringify({
      type: "outer",
      provider: provider?.provider ?? "",
      portal: url ?? "",
      entrypage: provider?.startPage ?? "/",
    }),
  );
}
