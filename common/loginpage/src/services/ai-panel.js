import { onCleanup } from "solid-js";
import { onNativeMessage } from "@/sdk/events";
import { htmlDecode } from "@/utils/html";
import { setPanelUrl } from "@/stores/aichat";
import { registerPlugin } from "@/stores/plugin-settings";

const AI_PLUGIN_IDS = ["F2402876-659F-47FB-A646-67B49F2B5AAA", "9DC93CDB-B576-4F0C-B55E-FCC9C48DD777"];

// native sends brace-wrapped GUIDs ({GUID}), so substring-match, not equality
const isAiPlugin = (id) => !!id && AI_PLUGIN_IDS.some((guid) => id.toUpperCase().includes(guid));

export function initAiPanel() {
  const unsubMessage = onNativeMessage((cmd, param) => {
    if (cmd !== "panel:external") return;
    let opts;
    try {
      opts = JSON.parse(htmlDecode(param));
    } catch {
      return;
    }
    if (isAiPlugin(opts?.id) && opts?.url) setPanelUrl(opts.url);
  });

  function onPluginEvent(msg = {}) {
    if (msg.type === "settings") {
      console.log("plugin settings", msg);
      registerPlugin(msg);
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
