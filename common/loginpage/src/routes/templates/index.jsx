import { onMount, onCleanup } from "solid-js";
import "../../../templatespanel/oforms.css";
import "./templates.css";

export default function Templates() {
  onMount(() => {
    OformsEmbed.render("#oforms-root", {locale: "en"});
  });

  return (
    <div class="stub-page">
      <div id="oforms-root" />
    </div>
  );
}
