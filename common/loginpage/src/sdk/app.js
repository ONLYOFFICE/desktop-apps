import { createSignal } from "solid-js";
import { command } from "./bridge";
import { onNativeMessage } from "./events";

export function signalAppReady() {
  command("app:onready", "");
}

const [appReady, setAppReady] = createSignal(false);
export { appReady };

onNativeMessage((msg) => {
  if (msg === "app:ready") setAppReady(true);
});
