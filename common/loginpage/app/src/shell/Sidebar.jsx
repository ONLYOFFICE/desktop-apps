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
      <MenuItem href="/templates" icon="#templates" label={t("actTemplates")} />
      <div id="idx-sidebar-portals" class="connect" />
      <li class="menu-item devider" />
      <Show when={promoVisible()}>
        <For each={promoButtons}>{(btn) => <MenuItem onClick={() => openPromo(btn.id)} icon={btn.icon} label={btn.text} />}</For>
      </Show>
      <MenuItem href="/settings/general" icon="#settings" label={t("actSettings")} />
      <MenuItem onClick={openAbout} icon="#about" label={t("actAbout")} hidden={!aboutVisible()} />
    </nav>
  );
}
