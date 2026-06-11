import { t } from "@/services/l10n";
import { setAichat } from "@/stores/aichat";
import styles from "./AiPanel.module.css";
import { Button } from "../button/Button.jsx";

export function AiTogglePill() {
  return (
    <Button type="button" variant="primary" class={styles.pill} onClick={() => setAichat("open", true)}>
      <svg class={styles.pillIcon} aria-hidden="true">
        <use href="#aichat" />
      </svg>
      {t("aiChatPillLabel")}
    </Button>
  );
}
