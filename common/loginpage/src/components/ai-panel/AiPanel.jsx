import { onCleanup, onMount } from "solid-js";
import { aichat } from "@/stores/aichat";
import { t } from "@/services/l10n";
import styles from "./AiPanel.module.css";

export function AiPanel() {
  let iframeEl;

  return (
    <div class={styles.panel}>
      <iframe
        id={"idx-plugin-frame"}
        ref={(el) => (iframeEl = el)}
        class={styles.frame}
        src={aichat.url || "about:blank"}
        title="AI"
        allow="clipboard-read; clipboard-write"
      />
    </div>
  );
}
