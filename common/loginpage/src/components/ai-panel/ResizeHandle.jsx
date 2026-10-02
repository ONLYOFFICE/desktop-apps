import { AI_CONTENT_MIN_WIDTH, AI_PANEL_MIN_WIDTH, setAichat } from "@/stores/aichat";
import styles from "./AiPanel.module.css";

// sidebar track (62 + 8) plus the two 4px shell grid gaps
const SHELL_LEFT = 78;

export function ResizeHandle() {
  let handleEl;
  let shellEdge = 0;
  let isRtl = false;
  let maxWidth = 0;
  let width = 0;

  function onMove(e) {
    const distance = isRtl ? e.clientX - shellEdge : shellEdge - e.clientX;
    width = Math.round(Math.min(Math.max(distance, AI_PANEL_MIN_WIDTH), maxWidth));
    document.body.style.setProperty("--chat-w", `${width}px`);
  }

  function onUp(e) {
    handleEl.releasePointerCapture(e.pointerId);
    handleEl.removeEventListener("pointermove", onMove);
    handleEl.removeEventListener("pointerup", onUp);
    handleEl.removeEventListener("pointercancel", onUp);
    document.body.classList.remove("chat-resizing");
    setAichat("width", width);
  }

  function onDown(e) {
    e.preventDefault();
    const rect = handleEl.closest(".shell").getBoundingClientRect();
    isRtl = document.body.dir === "rtl";
    shellEdge = isRtl ? rect.left : rect.right;
    maxWidth = Math.max(AI_PANEL_MIN_WIDTH, rect.width - SHELL_LEFT - AI_CONTENT_MIN_WIDTH);
    handleEl.setPointerCapture(e.pointerId);
    document.body.classList.add("chat-resizing");
    handleEl.addEventListener("pointermove", onMove);
    handleEl.addEventListener("pointerup", onUp);
    handleEl.addEventListener("pointercancel", onUp);
  }

  return <div ref={(el) => (handleEl = el)} class={styles.resizeHandle} aria-hidden="true" onPointerDown={onDown} />;
}
