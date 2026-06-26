import { For } from "solid-js";
import styles from "./layout.module.css";
import { t } from "@/services/l10n.js";
import { A, useLocation } from "@solidjs/router";
import { Button } from "@/components";
import { isOptsChanged, applySettings, pendingApplyText } from "@/services/settings-sync";
import { pluginConfigs } from "@/stores/plugin-settings";

export default function SettingsLayout(props) {
  const location = useLocation();

  const headerTitle = () => {
    const path = location.pathname;
    if (path.startsWith("/settings/plugin/")) {
      return decodeURIComponent(path.split("/").pop() ?? "");
    }
    return t("settingsGeneral");
  };

  const onApplyClick = e => {
    applySettings();
  };

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
          <For each={pluginConfigs()}>
            {(plugin) => (
              <For each={Object.keys(plugin.sections)}>
                {(sectionName) => (
                  <li class={styles.item}>
                    <A
                      href={`/settings/plugin/${encodeURIComponent(plugin.id)}/${encodeURIComponent(sectionName)}`}
                      activeClass={styles.selected}
                      end
                    >
                      {sectionName}
                    </A>
                  </li>
                )}
              </For>
            )}
          </For>
        </ul>
      </nav>
      <div class={styles.contentWrap}>
        <header class={styles.header}>
          <div class={styles.headerInner}>
            <h1 class={styles.headerTitle}>{headerTitle()}</h1>
          </div>
        </header>
        <main class={styles.content}>{props.children}</main>
        <footer class={styles.footer}>
          <div class={styles.footerInner}>
            <Button variant="accent" disabled={!isOptsChanged()} onClick={onApplyClick}>
              {pendingApplyText() ?? t("setBtnApply")}
            </Button>
            <span class={styles.footerNote}>{"* — " + t("settAfterRestart")}</span>
          </div>
        </footer>
      </div>
    </div>
  );
}
