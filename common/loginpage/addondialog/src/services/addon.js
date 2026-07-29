import { createSignal } from "solid-js";
import { command, onNativeMessage } from "./bridge";
import { htmlDecode } from "@/utils/html";
import { setViewPanel } from "@/App";

// "addon:action" (install/uninstall) and the "addon:status" status/progress
// event already exist in the native protocol (see cascapplicationmanagerwrapper.cpp
// and cmainwindow.cpp::sendAddonStatusToStartPage). "postpone"/"dismiss"/"restart"/"close"
// actions and "addon:resize" are NEW additions this dialog needs from the native side —
// there is no existing handler for them yet. "addon:status" is also expected to carry
// a "page" field ("intro"/"install"/"success"/"error") to drive which panel is shown,
// and, for "error", a "message" field with the error description.

const [addonStatus, setAddonStatus] = createSignal(null);
export { addonStatus };

export function installAddon() {
  command("addon:action", "install");
}

export function postponeAddon() {
  command("addon:action", "postpone");
}

export function dismissAddon() {
  command("addon:action", "dismiss");
}

export function restartNow() {
  command("addon:action", "restart");
}

export function closeDialog() {
  command("addon:action", "close");
}

export function resizeDialog(width, height) {
  command("addon:resize", { width, height });
}

export function initAddonStatus() {
  return onNativeMessage((cmd, param) => {
    if (cmd !== "addon:status") return;
    try {
      const status = JSON.parse(htmlDecode(param));
      if (status.progress == undefined && addonStatus().progress != undefined) {
        status.progress = addonStatus().progress;
      }

      setAddonStatus(status);

      if (status.page) {
        setViewPanel(status.page);
      } else if (!!status.progress) {
        console.log("progress", status.progress);
        /* progress event */
      }
    } catch {
      // ignore malformed payloads
    }
  });
}

export function progressPercentFromStatus(status) {
  return !!status?.progress ? Math.min(100, parseInt(status.progress, 10)) : null;
}
