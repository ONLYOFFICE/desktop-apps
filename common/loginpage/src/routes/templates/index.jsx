import { createSignal, onMount, onCleanup, Show } from "solid-js";
import { t, langCode } from "@/services/l10n";
import "./templates.css";
import { DocTypeGrid } from "../home/components/DocTypeGrid";

const TEMPLATES_URL = "https://oforms-new.pages.dev/";

let iframeEl = null;

export function sendTemplatesSettings(opts) {
  if (iframeEl?.contentWindow && opts.country) {
    iframeEl.contentWindow.postMessage({ type: "settings", country: opts.country }, "*");
  }
}

export default function Templates() {
  const [isOnline, setIsOnline] = createSignal(navigator.onLine);

  onMount(() => {
    const setOnline = () => setIsOnline(true);
    const setOffline = () => setIsOnline(false);
    window.addEventListener("online", setOnline);
    window.addEventListener("offline", setOffline);
    onCleanup(() => {
      window.removeEventListener("online", setOnline);
      window.removeEventListener("offline", setOffline);
    });
  });

  const site_url = new URL(TEMPLATES_URL);
  site_url.searchParams.set("hide", "lang");
  site_url.searchParams.set("lang", langCode);
  const src = site_url.toString();

  return (
    <div class="templates-page">
      <Show when={isOnline()}
        fallback={
          <div class="templates-offline-wrap">
            <div class="templates-offline-content">
              <h2 class="templates-title">{t("actTemplates")}</h2>
              <DocTypeGrid />
              <p class="templates-offline-msg">{t("tplNoInternet", "No internet connection")}</p>
            </div>
          </div>
        }
      >
        <h2 class="templates-title templates-title--online">{t("actTemplates")}</h2>
        <iframe
          ref={(el) => { iframeEl = el; }}
          id="idx-tmpls-frame"
          src={src}
          title={t("actTemplates")}
        />
      </Show>
    </div>
  );
}
