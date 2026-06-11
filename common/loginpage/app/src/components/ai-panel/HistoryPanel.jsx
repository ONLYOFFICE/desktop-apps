import { For, Show } from "solid-js";
import { t } from "@/services/l10n";
import { aichat, groupedHistory, setQuery, setView } from "@/stores/aichat";
import { HistoryListItem } from "./HistoryListItem.jsx";
import styles from "./AiPanel.module.css";

export function HistoryPanel() {
  const groups = () => groupedHistory();

  return (
    <>
      <header class={styles.header}>
        <span class={styles.title}>{t("aiChatHistoryTitle")}</span>
        <button type="button" class={styles.iconButton} aria-label={t("aiChatClose")} onClick={() => setView("chat")}>
          <svg class={styles.iconMd}>
            <use href="#ai-close" />
          </svg>
        </button>
      </header>

      <div class={styles.historyBody}>
        <div class={styles.searchBox}>
          <svg class={styles.searchIcon}>
            <use href="#ai-search" />
          </svg>
          <input
            class={styles.searchInput}
            type="text"
            placeholder={t("aiChatSearch")}
            value={aichat.query}
            onInput={(e) => setQuery(e.currentTarget.value)}
          />
        </div>

        <Show when={groups().length} fallback={<p class={styles.historyEmpty}>{t("aiChatNoHistory")}</p>}>
          <div class={styles.historyList}>
            <For each={groups()}>
              {(group) => (
                <div class={styles.historyGroup}>
                  <p class={styles.historyGroupLabel}>{t(group.labelKey)}</p>
                  <For each={group.chats}>{(chat) => <HistoryListItem chat={chat} />}</For>
                </div>
              )}
            </For>
          </div>
        </Show>
      </div>
    </>
  );
}
