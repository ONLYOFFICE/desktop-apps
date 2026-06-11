import { Show, createSignal } from "solid-js";
import { DropdownMenu } from "@/components";
import { mergeClass } from "@/utils/classes";
import { t } from "@/services/l10n";
import { aichat, clearAllChats, deleteChat, renameChat, selectChat } from "@/stores/aichat";
import { exportChat, turnIntoPrompt } from "@/services/chat-actions";
import styles from "./AiPanel.module.css";

export function HistoryListItem(props) {
  const [editing, setEditing] = createSignal(false);
  const isCurrent = () => aichat.currentId === props.chat.id;
  let inputRef;

  function commitRename() {
    if (inputRef) renameChat(props.chat.id, inputRef.value);
    setEditing(false);
  }

  function startRename() {
    setEditing(true);
    queueMicrotask(() => inputRef?.focus());
  }

  return (
    <div classList={mergeClass(styles.historyItem, isCurrent() ? styles.historyItemActive : undefined)}>
      <Show
        when={editing()}
        fallback={
          <button type="button" class={styles.historyItemTitle} onClick={() => selectChat(props.chat.id)}>
            {props.chat.title || t("aiChatUntitled")}
          </button>
        }
      >
        <input
          ref={(el) => (inputRef = el)}
          class={styles.historyRenameInput}
          value={props.chat.title}
          onBlur={commitRename}
          onKeyDown={(e) => {
            if (e.key === "Enter") commitRename();
            if (e.key === "Escape") setEditing(false);
          }}
        />
      </Show>

      <DropdownMenu>
        <DropdownMenu.Trigger class={styles.historyItemMenu} aria-label={t("aiChatItemMenu")}>
          <svg class={styles.smallIcon}>
            <use href="#ai-more" />
          </svg>
        </DropdownMenu.Trigger>
        <DropdownMenu.Content>
          <DropdownMenu.Item onSelect={() => selectChat(props.chat.id)}>{t("aiChatOpen")}</DropdownMenu.Item>
          <DropdownMenu.Item onSelect={() => turnIntoPrompt(props.chat.id)}>{t("aiChatTurnIntoPrompt")}</DropdownMenu.Item>
          <DropdownMenu.Item onSelect={() => exportChat(props.chat.id)}>{t("aiChatExportTo")}</DropdownMenu.Item>
          <DropdownMenu.Item icon="#ai-rename" onSelect={startRename}>
            {t("aiChatRename")}
          </DropdownMenu.Item>
          <DropdownMenu.Item class={styles.menuItemDanger} icon="#ai-delete" onSelect={() => deleteChat(props.chat.id)}>
            {t("aiChatDelete")}
          </DropdownMenu.Item>
          <DropdownMenu.Separator />
          <DropdownMenu.Item class={styles.menuItemDanger} icon="#ai-clear" onSelect={() => clearAllChats()}>
            {t("aiChatClearHistory")}
          </DropdownMenu.Item>
        </DropdownMenu.Content>
      </DropdownMenu>
    </div>
  );
}
