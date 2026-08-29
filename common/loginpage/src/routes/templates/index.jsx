import { onMount } from "solid-js";
import { settings } from "@/stores/settings";
import "./templates.css";
import { langCode } from "@/services/l10n";

const TEMPLATES_URL = "https://oforms-new.pages.dev/";

let iframeEl = null;

export function sendTemplatesSettings(opts) {
  if (iframeEl?.contentWindow && opts.country) {
    iframeEl.contentWindow.postMessage({ type: "settings", country: opts.country }, "*");
  }
}

export default function Templates() {
  const site_url = new URL(TEMPLATES_URL);
  site_url.searchParams.set("hide", "lang");
  site_url.searchParams.set("lang", langCode);

  // if (settings.country) src += `?country=${encodeURIComponent(settings.country)}`;

  return (
    <div class="stub-page">
      <iframe
        ref={(el) => { iframeEl = el; }}
        id="idx-tmpls-frame"
        src={site_url.toString()}
        title="Templates"
      />
    </div>
  );
}
