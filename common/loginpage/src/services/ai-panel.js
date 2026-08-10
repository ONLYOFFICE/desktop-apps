import { onCleanup } from "solid-js";
import { onNativeMessage } from "@/sdk/events";
import { htmlDecode } from "@/utils/html";
import { setPanelUrl, openSidePanel, closeSidePanel, setChatAvialable } from "@/stores/aichat";
import { registerPlugin, selectPluginSection, setPluginSectionsDisabled } from "@/stores/plugin-settings";
import { openPluginModal, closePluginModal } from "@/stores/plugin-modals";
import { addExtPlugin } from "@/stores/ext-plugins";

const AI_PLUGIN_IDS = ["F2402876-659F-47FB-A646-67B49F2B5AAA", "9DC93CDB-B576-4F0C-B55E-FCC9C48DD777"];

// native sends brace-wrapped GUIDs ({GUID}), so substring-match, not equality
const isAiPlugin = (id) => !!id && AI_PLUGIN_IDS.some((guid) => id.toUpperCase().includes(guid));

export function sendPluginVisibility(opts) {
  const iframe = document.getElementById("idx-plugin-frame");
  if (iframe && iframe.contentWindow) {
    let obj = {
      event: "onPluginEvent",
      data: {
        type: opts,// "minimized" || "expanded",
      }
    };
    iframe.contentWindow.postMessage(JSON.stringify(obj), "*");
  }
}

export function sendPluginSettings(opts) {
  const iframe = document.getElementById("idx-plugin-frame");
  if (iframe && iframe.contentWindow) {
    let obj = {
      event: "onPluginEvent",
      data: {
        type: "settings",
        items: opts
      }
    };
    iframe.contentWindow.postMessage(JSON.stringify(obj), "*");
  }
}
export function initAiPanel() {
  const unsubMessage = onNativeMessage((cmd, param) => {
    if (cmd !== "panel:external") return;
    let opts;
    try {
      opts = JSON.parse(htmlDecode(param));
    } catch {
      return;
    }
    if (isAiPlugin(opts?.id) && opts?.url) {
      setChatAvialable(true);
      setPanelUrl(opts.url);
    } else if (opts?.url) {
      addExtPlugin({ id: opts.id, name: opts.name || opts.id, url: opts.url });
    }
  });

  function onPluginEvent(msg = {}) {
    if (msg.type === "settings") {
      registerPlugin(msg);
    } else if (msg.type === "settings:select") {
      selectPluginSection(msg.id);
    } else if (msg.type === "settings:disable") {
      setPluginSectionsDisabled(msg.id, true);
      // window.location.hash = "#/settings/general";
    } else if (msg.type === "settings:enable") {
      setPluginSectionsDisabled(msg.id, false);
      // window.location.hash = "#/settings/general";
    } else if (msg.type === "settings:open") {
      window.location.hash = "#/settings";
    } else if (msg.type === "expand") {
      closeSidePanel();
      window.location.hash = "#/aichat";
    } else if (msg.type === "minimize") {
      openSidePanel();
      window.location.hash = "#/";
    } else if (msg.type === "modal:open") {
      openPluginModal(msg.id, msg.url, msg.width, msg.height);
    } else if (msg.type === "modal:close") {
      closePluginModal(msg.id);
    } else if (msg.type === "collapse") {
      closeSidePanel();
    }
  }

  function onMessage(e) {
    let msg;
    try {
      msg = typeof e.data === "string" ? JSON.parse(e.data) : e.data;
    } catch {
      return;
    }
    if (msg.event === "onPluginEvent") {
      onPluginEvent(msg.data);
    }
  }

  window.addEventListener("message", onMessage);

  onCleanup(() => {
    unsubMessage();
    window.removeEventListener("message", onMessage);
  });
}
