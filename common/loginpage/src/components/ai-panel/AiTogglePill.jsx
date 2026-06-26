import { t } from "@/services/l10n";
import { expandPanel } from "@/stores/aichat";
import styles from "./AiPanel.module.css";
import { Button } from "../button/Button.jsx";

export function AiTogglePill() {
  return (
    <Button type="button" variant="accent" class={styles.pill} onClick={() => expandPanel()}>
      <svg class={styles.pillIcon} aria-hidden="true">
        {/* TODO: change icon */}
        <use href="#aichat" />
      </svg>
      {t("aiChatPillLabel")}
    </Button>
  );
}
