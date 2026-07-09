import { command } from "./bridge.js";

// export const relpath = !/mac os/i.test(navigator.userAgent) ? "." : "..";
export const relpath = !/mac os/i.test(navigator.userAgent) ? ".." : "..";

export function getExternalClouds() {
  try {
    const raw = window.AscDesktopEditor.GetExternalClouds();
    return Array.isArray(raw) ? raw : [];
  } catch {
    return [];
  }
}

export function sendProviderList(list) {
  command("provider:list", JSON.stringify(list));
}

export function openPortal(portal, provider, entrypage) {
  command("portal:open", JSON.stringify({ portal, provider, entrypage }));
}

export function logoutPortal(domain, extra) {
  command("portal:logout", JSON.stringify({ domain, ...(extra ? { extra } : {}) }));
}

export function checkPortalsAuth(portals) {
  const sdk = window.AscDesktopEditor;
  sdk.checkAuth(portals);
}