import { createSignal } from "solid-js";
import { cmd } from "./bridge";
import { onNativeMessage } from "./events";

export function signalAppReady() {
  cmd("app:onready", "");
}

const [appReady, setAppReady] = createSignal(false);
export { appReady };

onNativeMessage((msg) => {
  if (msg === "app:ready") setAppReady(true);
});

// TODO: create in sdk/plugins.js
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