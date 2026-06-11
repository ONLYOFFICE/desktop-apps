import { copy } from "@/services/chat-actions";
import { t } from "@/services/l10n";
import styles from "./AiPanel.module.css";

export function ChatMessageActions(props) {
  // TODO(018+): Save (bookmark) and Sources need backend support — no-op for now
  const onSave = () => {};
  const onSources = () => {};

  return (
    <div class={styles.messageActions}>
      <button type="button" class={styles.messageActionButton} aria-label={t("aiChatCopy")} onClick={() => copy(props.message.text)}>
        <svg class={styles.smallIcon}>
          <use href="#ai-copy" />
        </svg>
      </button>
      <button
        type="button"
        class={styles.messageActionButton}
        aria-label={t("aiChatRegenerate")}
        onClick={() => props.onRegenerate?.(props.message.id)}
      >
        <svg class={styles.smallIcon}>
          <use href="#ai-regenerate" />
        </svg>
      </button>
      <button type="button" class={styles.messageActionButton} aria-label={t("aiChatSave")} onClick={onSave}>
        <svg class={styles.smallIcon}>
          <use href="#ai-save" />
        </svg>
      </button>
      <button type="button" class={styles.sourcesPill} onClick={onSources}>
        <svg class={styles.smallIcon}>
          <use href="#ai-sources" />
        </svg>
        <span>{t("aiChatSources")}</span>
      </button>
    </div>
  );
}
