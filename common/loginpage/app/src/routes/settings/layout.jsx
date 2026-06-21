import styles from "./layout.module.css";
import { t } from "@/services/l10n.js";
import { A } from "@solidjs/router";
import { Button } from "@/components";
import { isDirty, applySettings } from "@/services/settings-sync";

export default function SettingsLayout(props) {
  return (
    <div class={styles.layout}>
      <nav class={styles.sidebar}>
        <div class={styles.heading}>
          <span>{t("actSettings")}</span>
        </div>
        <ul class={styles.list}>
          <li class={styles.item}>
            <A href="/settings/general" activeClass={styles.selected} end>
              {t("settingsGeneral")}
            </A>
          </li>
        </ul>
      </nav>
      <div class={styles.contentWrap}>
        <main class={styles.content}>{props.children}</main>
        <footer class={styles.footer}>
          <div class={styles.footerInner}>
            <Button variant="accent" disabled={!isDirty()} onClick={applySettings}>
              {t("setBtnApply")}
            </Button>
            <span class={styles.footerNote}>{"* — " + t("settAfterRestart")}</span>
          </div>
        </footer>
      </div>
    </div>
  );
}
