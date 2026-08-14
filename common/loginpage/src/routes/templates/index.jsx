import { onMount, onCleanup } from "solid-js";
import "../../../templatespanel/oforms.css";
import "./templates.css";
import defaultTheme from "../../../templatespanel/theme.default.json";
import { settings } from "@/stores/settings";

export function sendTemplatesSettings(opts) {
  if ( OformsEmbed ) {
    if ( !!opts.country && OformsEmbed.setCountry )
      OformsEmbed.setCountry(opts.country);
  }
}

export default function Templates() {
  onMount(() => {
    OformsEmbed.render("#oforms-root", {locale: "en", theme: defaultTheme, country: settings.country});
  });

  return (
    <div class="stub-page">
      <div id="oforms-root" />
    </div>
  );
}
