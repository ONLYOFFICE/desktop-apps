import { AI_CONTENT_MIN_WIDTH, AI_PANEL_MIN_WIDTH, setAichat } from "@/stores/aichat";
import styles from "./AiPanel.module.css";

// sidebar track (62 + 8) plus the two 4px shell grid gaps
const SHELL_LEFT = 78;

export function ResizeHandle() {
  let handleEl;
  let shellRight = 0;
  let maxWidth = 0;
  let width = 0;

  function onMove(e) {
    width = Math.round(Math.min(Math.max(shellRight - e.clientX, AI_PANEL_MIN_WIDTH), maxWidth));
    document.documentElement.style.setProperty("--ai-w", `${width}px`);
  }

  function onUp(e) {
    handleEl.releasePointerCapture(e.pointerId);
    handleEl.removeEventListener("pointermove", onMove);
    handleEl.removeEventListener("pointerup", onUp);
    handleEl.removeEventListener("pointercancel", onUp);
    document.documentElement.classList.remove("ai-resizing");
    setAichat("width", width);
  }

  function onDown(e) {
    e.preventDefault();
    const rect = handleEl.closest(".shell").getBoundingClientRect();
    shellRight = rect.right;
    maxWidth = Math.max(AI_PANEL_MIN_WIDTH, rect.width - SHELL_LEFT - AI_CONTENT_MIN_WIDTH);
    handleEl.setPointerCapture(e.pointerId);
    document.documentElement.classList.add("ai-resizing");
    handleEl.addEventListener("pointermove", onMove);
    handleEl.addEventListener("pointerup", onUp);
    handleEl.addEventListener("pointercancel", onUp);
  }

  return <div ref={(el) => (handleEl = el)} class={styles.resizeHandle} aria-hidden="true" onPointerDown={onDown} />;
}
