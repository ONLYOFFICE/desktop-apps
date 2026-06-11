import { Button, DropdownMenu } from "@/components";
import { t } from "@/services/l10n";
import { aichat, deleteChat, setAichat, setView } from "@/stores/aichat";
import { exportChat, turnIntoPrompt } from "@/services/chat-actions";
import styles from "./AiPanel.module.css";

export function AiPanelHeader(props) {
  return (
    <header class={styles.header}>
      <div class={styles.titleGroup}>
        <span class={styles.title}>{t("aiChatTitle")}</span>
        <DropdownMenu>
          <DropdownMenu.Trigger class={styles.iconButton} aria-label={t("aiChatChatMenu")}>
            <svg class={styles.smallIcon}>
              <use href="#ai-chevron" />
            </svg>
          </DropdownMenu.Trigger>
          <DropdownMenu.Content>
            <DropdownMenu.Item onSelect={() => turnIntoPrompt(aichat.currentId)}>{t("aiChatTurnIntoPrompt")}</DropdownMenu.Item>
            <DropdownMenu.Item onSelect={() => exportChat(aichat.currentId)}>{t("aiChatExportTo")}</DropdownMenu.Item>
            <DropdownMenu.Separator />
            <DropdownMenu.Item class={styles.menuItemDanger} icon="#ai-delete" onSelect={() => deleteChat(aichat.currentId)}>
              {t("aiChatDeleteChat")}
            </DropdownMenu.Item>
          </DropdownMenu.Content>
        </DropdownMenu>
      </div>

      <div class={styles.headerActions}>
        <Button class={styles.newButton} variant="outline" onClick={() => props.onNewChat()}>
          <svg class={styles.smallIcon}>
            <use href="#ai-plus" />
          </svg>
          {t("aiChatNew")}
        </Button>
        <Button type="button" class={styles.iconButton} aria-label={t("aiChatHistory")} onClick={() => setView("history")}>
          <svg class={styles.iconMd}>
            <use href="#ai-history" />
          </svg>
        </Button>
        <Button type="button" class={styles.iconButton} aria-label={t("aiChatClose")} onClick={() => setAichat("open", false)}>
          <svg class={styles.iconMd}>
            <use href="#ai-close" />
          </svg>
        </Button>
      </div>
    </header>
  );
}
