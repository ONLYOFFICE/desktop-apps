import { For, Show } from "solid-js";
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

  const currentIframeKey = () => {
    const parts = location.pathname.split("/");
    if (parts[2] !== "plugin") return null;
    const pluginId = decodeURIComponent(parts[3] ?? "");
    const section = decodeURIComponent(parts[4] ?? "");
    const plugin = pluginConfigs().find((p) => p.id === pluginId);
    if (typeof plugin?.sections[section] !== "string") return null;
    return `${pluginId}/${section}`;
  };

  const isIframePage = () => currentIframeKey() !== null;

  const iframePages = () => {
    const pages = [];
    for (const plugin of pluginConfigs()) {
      for (const [section, value] of Object.entries(plugin.sections)) {
        if (typeof value === "string") {
          pages.push({ key: `${plugin.id}/${section}`, url: value });
        }
      }
    }
    return pages;
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
        <Show when={!isIframePage()}>
          <header class={styles.header}>
            <div class={styles.headerInner}>
              <h1 class={styles.headerTitle}>{headerTitle()}</h1>
            </div>
          </header>
        </Show>
        <main class={styles.content}>{props.children}</main>
        <Show when={!isIframePage()}>
          <footer class={styles.footer}>
            <div class={styles.footerInner}>
              <Button variant="accent" disabled={!isOptsChanged()} onClick={onApplyClick}>
                {pendingApplyText() ?? t("setBtnApply")}
              </Button>
              <span class={styles.footerNote}>{"* — " + t("settAfterRestart")}</span>
            </div>
          </footer>
        </Show>
        <For each={iframePages()}>
          {(page) => (
            <iframe
              src={page.url}
              class={styles.constIframe}
              classList={{ [styles.constIframeActive]: currentIframeKey() === page.key }}
            />
          )}
        </For>
      </div>
    </div>
  );
}
