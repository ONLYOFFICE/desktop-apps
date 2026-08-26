import { Switch, Match, createMemo } from "solid-js";
import { t } from "@/services/l10n";
import { addonStatus, progressPercentFromStatus, restartNow, closeDialog, postponeAddon } from "@/services/addon";
import logoSvg from "../../assets/img/logo.svg?raw";
import styles from "./StatusPanel.module.css";

export function StatusPanel(props) {
  const percent = createMemo(() => progressPercentFromStatus(addonStatus()));

  return (
    <div class={styles.panel}>
      <button type="button" class={styles.close} onClick={closeDialog} aria-label={t("addonBtnClose", "Close")} title={t("addonBtnClose", "Close")}>
        <span aria-hidden="true">&#x2715;</span>
      </button>
      <h1 class={styles.title}>{t("addonInstallingTitle", "AI Tools")}</h1>
      <div class={styles.icon} innerHTML={logoSvg} />

      <Switch>
        <Match when={props.page === "install"}>
          <p class={styles.status}>{addonStatus()?.message || t("addonInstalling")}</p>
          <div class={styles.progressTrack}>
            <div
              class={styles.progressBar}
              classList={{ [styles.indeterminate]: percent() == null }}
              style={percent() != null ? { width: `${percent()}%` } : undefined}
            />
          </div>
        </Match>

        <Match when={props.page === "complete"}>
          <p class={styles.message}>{addonStatus()?.message || t("addonInstalledTitle")}</p>
          <p class={styles.note}>{t("addonRestartNeeded")}</p>
          <div class={styles.actions}>
            <button type="button" class={styles.btnOutline} onClick={postponeAddon}>
              {t("addonBtnRestartLater")}
            </button>
            <button type="button" class={styles.btnPrimary} onClick={restartNow}>
              {t("addonBtnRestartNow")}
            </button>
          </div>
        </Match>

        <Match when={props.page === "error"}>
          <p class={`${styles.message} message`}>{t("addonInstallFailedTitle")}</p>
          <p class={`${styles.note} note`}>{addonStatus()?.message}</p>
          <div class={styles.devider}></div>
          <div class={styles.actions}>
            <button type="button" class={styles.btnOutline} onClick={closeDialog}>
              {t("addonBtnClose", "Close")}
            </button>
          </div>
        </Match>
      </Switch>
    </div>
  );
}
