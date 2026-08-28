import { onMount } from "solid-js";
import { settings } from "@/stores/settings";
import "./templates.css";
import { langCode } from "@/services/l10n";

const TEMPLATES_URL = "https://oforms-new.pages.dev/?hide=lang";

let iframeEl = null;

export function sendTemplatesSettings(opts) {
  if (iframeEl?.contentWindow && opts.country) {
    iframeEl.contentWindow.postMessage({ type: "settings", country: opts.country }, "*");
  }
}

export default function Templates() {
  let src = TEMPLATES_URL;
  // if (settings.country) src += `?country=${encodeURIComponent(settings.country)}`;

  return (
    <div class="stub-page">
      <iframe
        ref={(el) => { iframeEl = el; }}
        id="idx-tmpls-frame"
        src={src}
        title="Templates"
      />
    </div>
  );
}
