import { createSignal, onCleanup } from "solid-js";
import { onNativeMessage } from "@/sdk/events";
import { command } from "@/sdk/bridge";
import { htmlDecode } from "@/utils/html";
import { setChatAvialable } from "@/stores/aichat";

const [aboutOpen, setAboutOpen] = createSignal(false);
const [aboutVisible, setAboutVisible] = createSignal(false);
const [versionInfo, setVersionInfo] = createSignal(null);
const [updatesOn, setUpdatesOn] = createSignal(false);
const [updateStatus, setUpdateStatus] = createSignal(null);
const [addonInfo, setAddonInfo] = createSignal(null);

let _pendingUpdateAction = null;

export { aboutOpen, aboutVisible, versionInfo, updatesOn, updateStatus, addonInfo };

export function openAbout() {
  setAboutOpen(true);
}

export function closeAbout() {
  setAboutOpen(false);
}

export function execUpdateAction() {
  if (_pendingUpdateAction) command("updates:action", _pendingUpdateAction);
}

export function setAddonNeedRestart(restart) {
  if (addonInfo()?.restart != restart) {
    const info = addonInfo();
    info.restart = restart;
    setAddonInfo(info);
  }
}

export function initAbout() {
  const unsub = onNativeMessage((cmd, param) => {
    if (/app:version/.test(cmd)) {
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
    } else if (/addon:status/.test(cmd)) {
      try {
        const info = JSON.parse(htmlDecode(param));
        if (info?.version) {
          info.version = info.version.replace(/^\d+/, m => {const mn = parseInt(m);return mn<10?1:mn-9});
        }

        if (info.installed == undefined && addonInfo()?.installed)
          info.installed = addonInfo().installed;

        if (info.restart == undefined && addonInfo()?.restart)
          info.restart = addonInfo().restart;

        setAddonInfo(info);
        setChatAvialable(info?.installed || info?.integrated);
      } catch {}
    } else if (/^updates:turn/.test(cmd)) {
      setUpdatesOn(param === "on");
    } else if (/updates:status/.test(cmd)) {
      try {
        const info = JSON.parse(param);
        if (updateStatus()?.button) {
          !info.button && (info.button = {});
          if (!info.button.text && updateStatus().button.text) {
            info.button.text = updateStatus().button.text;
          }
          if (info.button.lock===undefined && updateStatus().button.lock!==undefined) {
            info.button.lock = updateStatus().button.lock;
          }
        }
        setUpdateStatus(info);
        if (info.button?.action) _pendingUpdateAction = info.button.action;
      } catch {}
    }
  });

  onCleanup(unsub);
}
