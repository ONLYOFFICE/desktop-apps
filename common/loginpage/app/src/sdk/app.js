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
