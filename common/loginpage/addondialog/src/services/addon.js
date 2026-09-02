import { createSignal } from "solid-js";
import { command, onNativeMessage } from "./bridge";
import { htmlDecode } from "@/utils/html";
import { Downloader } from "../../../src/utils/downloader";
import { setViewPanel } from "@/App";
import { t } from "./l10n";

// "addon:action" (install/uninstall) and the "addon:status" status/progress
// event already exist in the native protocol (see cascapplicationmanagerwrapper.cpp
// and cmainwindow.cpp::sendAddonStatusToStartPage). "postpone"/"dismiss"/"restart"/"close"
// actions and "addon:resize" are NEW additions this dialog needs from the native side —
// there is no existing handler for them yet. "addon:status" is also expected to carry
// a "page" field ("intro"/"install"/"success"/"error") to drive which panel is shown,
// and, for "error", a "message" field with the error description.

const [addonStatus, setAddonStatus] = createSignal(null);
export { addonStatus };

let activeDownload = null;
let activeRequest = null;

export function installAddon() {
  command("addon:action", "install");
}

export function postponeAddon() {
  command("addon:action", "postpone");
}

export function dismissAddon() {
  command("addon:action", "disable");
}

export function restartNow() {
  command("addon:action", "restart");
}

export function closeDialog() {
  if (activeRequest) {
    activeRequest.canceled = true;
    activeRequest.controller.abort();
    activeRequest = null;
  }

  if (activeDownload) {
    activeDownload.canceled = true;
    activeDownload.downloader.abort();
    activeDownload = null;
  }

  command("addon:action", "close");
}

export function resizeDialog(width, height) {
  command("addon:resize", { width, height });
}

function downloadAddon(url, size, md5) {
  const operation = { canceled: false, downloader: null };
  const downloader = new Downloader({
    url,
    // fileName,
    size,
    onProgress: ({ percent }) => {
      if (operation.canceled) return;
      setAddonStatus((prev) => ({ ...(prev ?? {}), progress: percent }));
    },
    onComplete: (filePath) => {
      if (operation.canceled) return;
      if (md5 !== undefined) {
        setAddonStatus((prev) => ({ ...(prev ?? {}), message: t("addonInstalling") }));
        command("addon:install", { status: "downloaded", tempfile: filePath, md5 });
      }
    },
    onError: (err) => {
      if (operation.canceled) return;
      setViewPanel("error");
      setAddonStatus((prev) => ({ ...(prev ?? {}), message: err.message }));
    },
  });
  operation.downloader = downloader;
  activeDownload = operation;
  downloader.start().catch(() => {
    // already surfaced via onError
  }).finally(() => {
    if (activeDownload === operation) activeDownload = null;
  });
}

function compareVersions(v1, v2) {
    const parts1 = v1.split('.').map(Number);
    const parts2 = v2.split('.').map(Number);
    const maxLength = Math.max(parts1.length, parts2.length);

    for (let i = 0; i < maxLength; i++) {
        const num1 = parts1[i] || 0;
        const num2 = parts2[i] || 0;

        if (num1 > num2) return 1;  // v1 is greater
        if (num1 < num2) return -1; // v2 is greater
    }
    return 0; // Versions are equal
}

export async function startInstall({ appcast, package: pkg, version: app_version }) {
  if (!appcast || !appcast.length) {
    setViewPanel("error");
    setAddonStatus((prev) => ({ ...(prev ?? {}), message: 'appcast fetch failed: url is empty' }));
    return;
  }

  let request;
  try {
    request = { canceled: false, controller: new AbortController() };
    activeRequest = request;
    const res = await fetch(`onlyoffice-proxy://${appcast}`, {
      signal: request.controller.signal,
      cache: "no-cache",
    });
    if (activeRequest === request) activeRequest = null;
    if (!res.ok)
      throw new Error(`appcast fetch failed: ${res.status}`);

    const json = await res.json();
    const server_version = json?.addon?.version;
    if (!!server_version && !!app_version) {
      const addon_version = server_version.replace(/^\d+/, m => { const mn = parseInt(m); return mn < 10 ? 1 : mn - 9; });
      const cmpr = compareVersions(server_version, app_version);
      if (cmpr > 0) {
        throw new Error(t("errorInstallVersionHigh").replace("%1", server_version));
      } else if (cmpr < 0) {
        throw new Error(t("errorInstallVersionLow")
          .replace("%1", addon_version)
          .replace("%mail", '<a href="mailto:support@onlyoffice.com">support team</a>')
        );
      }
    } else if (!server_version || !app_version) {
      throw new Error(t("errorInstallVersionUnknown")
        .replace("%mail", '<a href="mailto:support@onlyoffice.com">support team</a>'));
    }

    const pkgBlock = json?.addon?.package?.[pkg];
    if (!pkgBlock?.url) throw new Error(`package "${pkg}" not found in appcast`);
    setAddonStatus((prev) => ({ ...(prev ?? {}), message: t("addonDownloading") }));
    downloadAddon(pkgBlock.url, pkgBlock.size, pkgBlock.md5);
  } catch (err) {
    if (request?.canceled) return;
    setViewPanel("error");
    setAddonStatus((prev) => ({ ...(prev ?? {}), message: err.message }));
  } finally {
    if (activeRequest === request) activeRequest = null;
  }
}

export function initAddonStatus() {
  return onNativeMessage(async (cmd, param) => {
    if (cmd !== "addon:status") return;
    try {
      const status = JSON.parse(htmlDecode(param));
      if (status.progress == undefined && addonStatus()?.progress != undefined) {
        status.progress = addonStatus().progress;
      }

      if (addonStatus()?.page !== "error" || status.page === "error")
        setAddonStatus(status);

      if (status.page) {
        setViewPanel(status.page);
        if (status.page === "install") {
          startInstall(status);
        }
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
