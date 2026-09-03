import { onCleanup } from "solid-js";
import { onNativeMessage } from "@/sdk/events";
import { htmlDecode } from "@/utils/html";
import { setPanelUrl, openSidePanel, closeSidePanel, setChatAvialable, setAichat } from "@/stores/aichat";
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

export function sendFileToChat(path, type, name) {
  const iframe = document.getElementById("idx-plugin-frame");
  if (iframe && iframe.contentWindow) {
    let file = {path: path};
    type != undefined && (file.type = type);
    name != undefined && (file.title = name);

    const obj = {
      event: "onPluginEvent",
      data: {
        type: "attach:file",
        // requestId: "",
        files: [file],
      }
    };
    iframe.contentWindow.postMessage(JSON.stringify(obj), "*");
  }
}

export function sendTextToChat(t) {
  const iframe = document.getElementById("idx-plugin-frame");
  if (iframe && iframe.contentWindow) {
    const obj = {
      event: "onPluginEvent",
      data: {
        type: "attach:text",
        text: t,
        forseSend: false,
      }
    };
    iframe.contentWindow.postMessage(JSON.stringify(obj), "*");
  }
}

function chatMessage(data) {
  const iframe = document.getElementById("idx-plugin-frame");
  if (iframe && iframe.contentWindow) {
    const obj = {
      event: "onPluginEvent",
      data: data
    };
    iframe.contentWindow.postMessage(JSON.stringify(obj), "*");
  }
}

export function sendChatRecentsChanged() {
  chatMessage({
    type: "recents:changed",
  });
}

let chatReady = false;
const pendingStatuses = new Map();

export function sendFileStatusToChat(model) {
  if (!chatReady) {
    pendingStatuses.set(model.id, model);
    return;
  }
  chatMessage({
    type: 'recents:available',
    files: [{ exist: model.exist, id: model.id, path: model.path }],
  });
}

function flushPendingStatuses() {
  if (pendingStatuses.size === 0) return;
  chatMessage({
    type: 'recents:available',
    files: [...pendingStatuses.values()].map(m => ({ exist: m.exist, id: m.id, path: m.path })),
  });
  pendingStatuses.clear();
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
      setAichat("expanded",true);
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
      document.getElementById("idx-plugin-frame")?.blur();
    } else if (msg.type === "ai-plugin:ready") {
      chatReady = true;
      flushPendingStatuses();
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
