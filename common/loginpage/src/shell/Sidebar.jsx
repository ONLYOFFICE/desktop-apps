import { For, Show } from "solid-js";
import { t } from "@/services/l10n";
import { openAbout, aboutVisible } from "@/services/about";
import { openPromo, promoVisible, promoButtons } from "@/services/promotion";
import { aichat, expandChatPanel } from "@/stores/aichat";
import { appliedUseAi } from "@/services/settings-sync";
import { MenuItem } from "./MenuItem.jsx";
import "./Sidebar.styles.css";
import { command } from "@/sdk/bridge.js";

const clickAiChat = () => {
  if (aichat.available) {
    expandChatPanel();
  } else {
    command('addon', 'advert');
  }
};

export default function Sidebar() {
  return (
    <nav class="tool-menu">
      <Show when={appliedUseAi()}>
        <MenuItem onClick={clickAiChat} icon="#aichat" label={t("actAiChat")} />
        <li class="menu-item separator" />
      </Show>
      <MenuItem href="/" icon="#home" label={t("actHome")} end />
      <MenuItem href="/open" icon="#folder" label={t("actOpenLocal")} />
      <MenuItem action="templates" href="/templates" icon="#templates" label={t("actTemplates")} />
      <MenuItem action="clouds" href="/clouds" icon="#cloud" label={t("actClouds")} />
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
