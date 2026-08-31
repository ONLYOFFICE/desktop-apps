import { createMemo, onMount } from "solid-js";
import { t } from "@/services/l10n";
import { Carousel } from "./Carousel";
import { postponeAddon, dismissAddon, closeDialog } from "@/services/addon";
import styles from "./IntroPanel.module.css";

const LICENSE_URL = "https://docspace.onlyoffice.com/s/jPfGKDyBYTRK4JM";

export function IntroPanel(props) {
  const licenseParts = createMemo(() => t("addonLicenseText").split("$1"));
  let panel;

  onMount(() => panel.focus());

  const handleKeyDown = (event) => {
    if (event.key === "Escape") {
      event.preventDefault();
      closeDialog();
    }
  };

  return (
    <div ref={panel} class={styles.panel} tabIndex="-1" onKeyDown={handleKeyDown}>
      <h1 class={styles.title}>{t("addonTitle")}</h1>
      <p class={styles.descr}>{t("addonDescr")}</p>

      <Carousel />

      <div class={styles.devider}></div>

      <p class={styles.license}>
        {licenseParts()[0]}
        {/* <br/> */}
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
