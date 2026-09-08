import { createSignal, onMount, onCleanup, Show } from "solid-js";
import { t, langCode } from "@/services/l10n";
import "./templates.css";
import { DocTypeGrid } from "../home/components/DocTypeGrid";

const TEMPLATES_URL_1 = "https://oforms-new.pages.dev/";
const TEMPLATES_URL_2 = "https://oforms-new.site.dev/";

let iframeEl = null;

export function sendTemplatesSettings(opts) {
  if (iframeEl?.contentWindow && opts) {
    iframeEl.contentWindow.postMessage(opts, "*");
  }
}

export function sendTemplatesCountry(code) {
  if (iframeEl?.contentWindow && code) {
    // sendTemplatesSettings({type: "country", value: code});
    sendTemplatesSettings({type: "locale", value: code});
  }
}

export function sendTemplatesTheme(themeType) {
  if (iframeEl?.contentWindow) {
    iframeEl.contentWindow.postMessage({ type: "theme", value: themeType }, "*");
  }
}

function makeIframeUrl(base) {
  const u = new URL(base);
  // u.searchParams.set("hide", "lang");
  u.searchParams.set("lang", langCode());
  return u.toString();
}

export default function Templates() {
  const [isOnline, setIsOnline] = createSignal(navigator.onLine);
  const [src, setSrc] = createSignal(makeIframeUrl(TEMPLATES_URL_1));
  let triedFallback = false;

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

  const _onload = function() {
    if (triedFallback) return;
    let blocked = false;
    try {
      blocked = iframeEl.contentWindow.location.href === 'about:blank';
    } catch {
      blocked = false; // SecurityError = a real cross-origin doc rendered
    }
    if (blocked) {
      triedFallback = true;
      // setSrc(makeIframeUrl(TEMPLATES_URL_2));
      setSrc(makeIframeUrl(TEMPLATES_URL_1));
    }
  };

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
          src={src()}
          title={t("actTemplates")}
          onload={_onload}
        />
      </Show>
    </div>
  );
}
