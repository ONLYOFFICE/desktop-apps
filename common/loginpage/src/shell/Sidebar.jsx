import { For, Show } from "solid-js";
import { t } from "@/services/l10n";
import { extPlugins, pluginRouteId } from "@/stores/ext-plugins";
import { openAbout, aboutVisible } from "@/services/about";
import { openPromo, promoVisible, promoButtons } from "@/services/promotion";
import { appliedUseAi } from "@/services/settings-sync";
import { MenuItem } from "./MenuItem.jsx";
import { command } from "@/sdk/bridge.js";
import { aichat, closeSidePanel } from "@/stores/aichat.js";
import "./Sidebar.styles.css";
import { sendPluginVisibility } from "@/services/ai-panel.js";

export default function Sidebar() {
  const clickAiChat = (e) => {
    if (aichat.available) {
      closeSidePanel();
      sendPluginVisibility("expanded");
      // href handles navigation
    } else {
      e.preventDefault();
      command('addon:action', 'intro');
    }
  };

  const clickAiTools = (e) => {
    if (aichat.available) {
      closeSidePanel();
      // href handles navigation
    } else {
      e.preventDefault();
      command('addon:action', 'intro');
    }
  };

  return (
    <nav class="tool-menu">
      <Show when={appliedUseAi()}>
        <MenuItem href="/ai" onClick={clickAiChat} icon="#aichat" label={t("actAiChat")} />
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
      <Show when={promoVisible()}>
        <For each={promoButtons}>{
          (btn) => <MenuItem class={btn.cls} onClick={() => openPromo(btn.id)}
                            icon={btn.icon} label={btn.text} />
        }
        </For>
      </Show>
      {/* <div class="menu-item separator" /> */}
      <MenuItem href="/settings/general" icon="#settings" label={t("actSettings")} action="settings" class="cls-btn" />
      <MenuItem onClick={openAbout} icon="#about" label={t("actAbout")} hidden={!aboutVisible()} />
    </nav>
  );
}
