import { For, Show, createSignal, createEffect } from "solid-js";
import { useLocation } from "@solidjs/router";
import { t } from "@/services/l10n";
import { extPlugins, pluginRouteId } from "@/stores/ext-plugins";
import { openAbout, aboutVisible, addonInfo } from "@/services/about";
import { appliedUseAi } from "@/services/settings-sync";
import { MenuItem } from "./MenuItem.jsx";
import { command } from "@/sdk/bridge.js";
import { aichat, closeSidePanel, openFullAIPanel } from "@/stores/aichat.js";
import "./Sidebar.styles.css";
// import { sendPluginVisibility } from "@/services/ai-panel.js";

export default function Sidebar() {
  const location = useLocation();
  const isSettingsActive = () => location.pathname.startsWith("/settings");
  const [settingsHref, setSettingsHref] = createSignal("/settings/general");
  createEffect(() => { if (isSettingsActive()) setSettingsHref(location.pathname); });

  const clickAiChat = (e) => {
    if (aichat.available) {
      if (addonInfo().restart === true) {
        e.preventDefault();
        command('addon:action', 'askrestart');
      } else {
        closeSidePanel();
        openFullAIPanel();
        // sendPluginVisibility("expanded");
        // href handles navigation
      }
    } else {
      e.preventDefault();
      command('addon:action', 'intro');
    }
  };

  const clickAiTools = (e) => {
    if (aichat.available) {
      if (addonInfo().restart === true) {
        e.preventDefault();
        command('addon:action', 'askrestart');
      }
      // closeSidePanel();
      // href handles navigation
    } else {
      e.preventDefault();
      command('addon:action', 'intro');
    }
  };

  return (
    <nav class="tool-menu">
      <Show when={appliedUseAi()}>
        <MenuItem href="/aichat" onClick={clickAiChat} icon="#aichat" label={t("actAiChat")} />
        <li class="menu-item separator" />
      </Show>
      <MenuItem href="/" icon="#home" label={t("actHome")} end />
      <MenuItem href="/open" icon="#folder" label={t("actOpenLocal")} />
      <MenuItem action="templates" href="/templates" icon="#templates" label={t("actTemplates")} />
      <MenuItem action="clouds" href="/clouds" icon="#cloud" label={t("actClouds")} />
      <Show when={appliedUseAi()}>
        <MenuItem href="/aitools" onClick={clickAiTools} icon="#aitools" label={t("actAiTools", "AI Tools")} />
      </Show>
      <For each={extPlugins()}>
        {(plugin) => (
          <MenuItem
            href={`/plugin/${pluginRouteId(plugin.id)}`}
            icon="#plugin"
            label={plugin.name}
          />
        )}
      </For>
      <li class="menu-item devider" />
      <MenuItem href={settingsHref()} icon="#settings" label={t("actSettings")} action="settings" class="cls-btn" isActive={isSettingsActive} />
      <MenuItem onClick={openAbout} icon="#about" label={t("actAbout")} hidden={!aboutVisible()} />
    </nav>
  );
}
