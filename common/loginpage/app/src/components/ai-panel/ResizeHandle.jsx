import { onCleanup } from "solid-js";
import { AI_CONTENT_MIN_WIDTH, AI_PANEL_MIN_WIDTH, setAichat } from "@/stores/aichat";
import styles from "./AiPanel.module.css";

// sidebar track (62 + 8) plus the two 4px shell grid gaps
const SHELL_LEFT = 78;

export function ResizeHandle() {
  let handleEl;
  let dragging = false;

  function clampWidth(w) {
    const shellWidth = handleEl?.closest(".shell")?.clientWidth ?? window.innerWidth;
    const max = Math.max(AI_PANEL_MIN_WIDTH, shellWidth - SHELL_LEFT - AI_CONTENT_MIN_WIDTH);
    return Math.round(Math.min(Math.max(w, AI_PANEL_MIN_WIDTH), max));
  }

  const widthFromEvent = (e) => {
    const right = handleEl?.closest(".shell")?.getBoundingClientRect().right ?? window.innerWidth;
    return clampWidth(right - e.clientX);
  };

  function onMove(e) {
    if (!dragging) return;
    document.documentElement.style.setProperty("--ai-w", `${widthFromEvent(e)}px`);
  }

  function onUp(e) {
    if (!dragging) return;
    dragging = false;
    setAichat("width", widthFromEvent(e));
    window.removeEventListener("pointermove", onMove);
    window.removeEventListener("pointerup", onUp);
  }

  function onDown(e) {
    e.preventDefault();
    dragging = true;
    window.addEventListener("pointermove", onMove);
    window.addEventListener("pointerup", onUp);
  }

  onCleanup(() => {
    window.removeEventListener("pointermove", onMove);
    window.removeEventListener("pointerup", onUp);
  });

  return <div ref={(el) => (handleEl = el)} class={styles.resizeHandle} aria-hidden="true" onPointerDown={onDown} />;
}
