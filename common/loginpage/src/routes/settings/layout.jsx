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
    const parts = location.pathname.split("/");
    if (parts[2] !== "plugin") return t("settingsGeneral");
    const pluginId = decodeURIComponent(parts[3] ?? "");
    const sectionId = decodeURIComponent(parts[4] ?? "");
    const plugin = pluginConfigs().find((p) => p.id === pluginId);
    const section = plugin?.sections.find((s) => s.id === sectionId);
    return section?.text ?? t("settingsGeneral");
  };

  const currentIframeKey = () => {
    const parts = location.pathname.split("/");
    if (parts[2] !== "plugin") return null;
    const pluginId = decodeURIComponent(parts[3] ?? "");
    const sectionId = decodeURIComponent(parts[4] ?? "");
    const plugin = pluginConfigs().find((p) => p.id === pluginId);
    const section = plugin?.sections.find((s) => s.id === sectionId);
    if (!section?.url) return null;
    return `${pluginId}/${sectionId}`;
  };

  const isIframePage = () => currentIframeKey() !== null;

  const iframePages = () => {
    const pages = [];
    for (const plugin of pluginConfigs()) {
      for (const section of plugin.sections) {
        if (section.url) {
          pages.push({ key: `${plugin.id}/${section.id}`, url: section.url });
        }
      }
    }
    return pages;
  };

  const onApplyClick = () => {
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
              <For each={plugin.sections}>
                {(section) => (
                  <li class={styles.item}>
                    <Show
                      when={!section.disabled}
                      fallback={<span class={styles.itemDisabled}>{section.text}</span>}
                    >
                      <A
                        href={`/settings/plugin/${encodeURIComponent(plugin.id)}/${encodeURIComponent(section.id)}`}
                        activeClass={styles.selected}
                        end
                      >
                        {section.text}
                      </A>
                    </Show>
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
