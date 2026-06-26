import { createSignal, onCleanup } from "solid-js";
import { onNativeMessage } from "@/sdk/events";
import { cmd } from "@/sdk/bridge";
import { htmlDecode } from "@/utils/html";

const [aboutOpen, setAboutOpen] = createSignal(false);
const [aboutVisible, setAboutVisible] = createSignal(false);
const [versionInfo, setVersionInfo] = createSignal(null);
const [updatesOn, setUpdatesOn] = createSignal(false);
const [updateStatus, setUpdateStatus] = createSignal(null);

let _pendingUpdateAction = null;

export { aboutOpen, aboutVisible, versionInfo, updatesOn, updateStatus };

export function openAbout() {
  setAboutOpen(true);
}

export function closeAbout() {
  setAboutOpen(false);
}

export function execUpdateAction() {
  if (_pendingUpdateAction) cmd("updates:action", _pendingUpdateAction);
}

export function initAbout() {
  const unsub = onNativeMessage((сmd, param) => {
    if (/app:version/.test(сmd)) {
      try {
        const opts = JSON.parse(htmlDecode(param));
        if (!opts.site && opts.link) opts.site = opts.link.replace(/^https?:\/\//, "");
        if (opts.active) opts.edition = opts.edition ? `${opts.edition} ${opts.active}` : opts.active;
        let ext = "";
        if (opts.arch) ext += opts.arch;
        if (opts.pkg) ext += ` ${opts.pkg}`;
        if (ext.trim()) opts.version += ` (${ext.trim()})`;
        setVersionInfo(opts);
        setAboutVisible(true);
      } catch {}
    } else if (/^updates:turn/.test(сmd)) {
      setUpdatesOn(param === "on");
    } else if (/updates:status/.test(сmd)) {
      try {
        const info = JSON.parse(param);
        setUpdateStatus(info);
        if (info.button?.action) _pendingUpdateAction = info.button.action;
      } catch {}
    }
  });

  onCleanup(unsub);
}
