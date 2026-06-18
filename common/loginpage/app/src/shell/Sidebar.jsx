import { For, Show } from "solid-js";
import { t } from "@/services/l10n";
import { openAbout, aboutVisible } from "@/services/about";
import { openPromo, promoVisible, promoButtons } from "@/services/promotion";
import { MenuItem } from "./MenuItem.jsx";
import "./Sidebar.styles.css";

export default function Sidebar() {
  return (
    <nav class="tool-menu">
      <MenuItem href="/" icon="#home" label={t("actHome")} end />
      <MenuItem href="/open" icon="#folder" label={t("actOpenLocal")} />
      <MenuItem action="templates" href="/templates" icon="#templates" label={t("actTemplates")} />
      <MenuItem action="clouds" href="/clouds" icon="#cloud" label={t("actClouds")} />
      <li class="menu-item devider" />
      <Show when={promoVisible()}>
        <For each={promoButtons}>{
          (btn) => <MenuItem class="promo-btn" onClick={() => openPromo(btn.id)}
                            icon={btn.icon} label={btn.text} />
        }
        </For>
      </Show>
      <div class="menu-item separator" />
      <MenuItem href="/settings/general" icon="#settings" label={t("actSettings")} action="settings" class="cls-btn" />
      <MenuItem onClick={openAbout} icon="#about" label={t("actAbout")} hidden={!aboutVisible()} />
    </nav>
  );
}
