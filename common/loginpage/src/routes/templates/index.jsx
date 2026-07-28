import { onMount, onCleanup } from "solid-js";
import "../../../templatespanel/oforms.css";
import "./templates.css";
import defaultTheme from "../../../templatespanel/theme.default.json";

export default function Templates() {
  onMount(() => {
    OformsEmbed.render("#oforms-root", {locale: "en", theme: defaultTheme});
  });

  return (
    <div class="stub-page">
      <div id="oforms-root" />
    </div>
  );
}
