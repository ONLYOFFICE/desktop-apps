import { command } from "./bridge.js";
import { uuid } from "@/utils/uuid.js";

export const relpath = !/mac os/i.test(navigator.userAgent) /*&& import.meta.env.PROD*/ ? "." : "..";

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

export function keepPortalCookie(portal) {
  const match = /^(https?:\/{2})?([^<>/\s]+)(\/[^\s]+)?/i.exec(portal);
  if (!match) return false;
  window.AscDesktopEditor.setCookie(portal, match[2], match[3] || "/", "asc_auth_key", uuid());
  return true;
}

export function checkPortalsAuth(portals) {
  const sdk = window.AscDesktopEditor;
  sdk.checkAuth(portals);
}