import { onCleanup, onMount } from "solid-js";
import { aichat, collapsePanel, expandPanel } from "@/stores/aichat";
import { t } from "@/services/l10n";
import { ResizeHandle } from "./ResizeHandle.jsx";
import styles from "./AiPanel.module.css";

export function AiPanel() {
  let iframeEl;

  function onMessage(e) {
    if (!iframeEl || e.source !== iframeEl.contentWindow) return;
    let msg;
    try {
      msg = typeof e.data === "string" ? JSON.parse(e.data) : e.data;
    } catch {
      return;
    }
    if (msg?.type !== "plugin") return;
    if (msg.event === "panel:collapse") collapsePanel();
    else if (msg.event === "panel:expand") expandPanel();
  }

  onMount(() => window.addEventListener("message", onMessage));
  onCleanup(() => window.removeEventListener("message", onMessage));

  return (
    <div class={styles.panel}>
      <ResizeHandle />
      <button type="button" class={styles.collapseButton} aria-label={t("aiChatClose")} onClick={() => collapsePanel()}>
        <svg class={styles.collapseIcon} aria-hidden="true">
          <use href="#ai-close" />
        </svg>
      </button>
      <iframe
        ref={(el) => (iframeEl = el)}
        class={styles.frame}
        src={aichat.url || "about:blank"}
        title="AI"
        allow="clipboard-read; clipboard-write"
      />
    </div>
  );
}
