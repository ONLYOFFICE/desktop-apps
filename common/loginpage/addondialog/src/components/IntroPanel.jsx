import { createMemo } from "solid-js";
import { t } from "@/services/l10n";
import { Carousel } from "./Carousel";
import { postponeAddon, dismissAddon } from "@/services/addon";
import styles from "./IntroPanel.module.css";

const LICENSE_URL = "https://testinfo.eu/license";

export function IntroPanel(props) {
  const licenseParts = createMemo(() => t("addonLicenseText").split("$1"));

  return (
    <div class={styles.panel}>
      <h1 class={styles.title}>{t("addonTitle")}</h1>
      <p class={styles.descr}>{t("addonDescr")}</p>

      <Carousel />

      <div class={styles.devider}></div>

      <p class={styles.license}>
        {licenseParts()[0]}
        <br/>
        <a class={styles.licenseLink} href={LICENSE_URL} target="_blank" rel="noreferrer">
          {t("addonLicenseLink")}
        </a>
        {licenseParts()[1]}
      </p>

      <div class={styles.actions}>
        <button type="button" class={styles.btnOutline} onClick={postponeAddon}>
          {t("addonBtnLater")}
        </button>
        <button type="button" class={styles.btnPrimary} onClick={props.onInstall}>
          {t("addonBtnInstall")}
        </button>
      </div>

      <button type="button" class={styles.dontShow} onClick={dismissAddon}>
        {t("addonDontShow")}
      </button>
    </div>
  );
}
