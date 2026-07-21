import { createSignal } from "solid-js";
import { inputArgs } from "@/services/url";

// adapted from loginpage/src/services/l10n.js: locale tables are bundled via
// import.meta.glob instead of relying on external <script> tags setting
// window.l10n, since this project builds to a single standalone html file.
const localeModules = import.meta.glob("../../locale/*.js", { eager: true });

const l10n = {};
for (const path in localeModules) {
  const code = path.match(/([^/]+)\.js$/)[1];
  l10n[code.replaceAll("-", "_")] = localeModules[path].default;
}

const [translations, setTranslations] = createSignal({});

export function initLocale() {
  setTranslations({ ...(l10n.en ?? {}) });

  const lang = inputArgs()?.lang;
  if (lang) applyLocale(lang);
}

export function t(key, defstring) {
  return translations()[key] ?? defstring ?? key;
}

export function applyLocale(locale) {
  const correct = locale.replaceAll("-", "_");
  let table = l10n[correct];

  if (!table) {
    const code = /^\w{2}/.exec(correct)?.[0];
    if (code) {
      for (const l in l10n) {
        if (l.startsWith(code)) {
          table = l10n[l];
          break;
        }
      }
    }
  }

  if (table) setTranslations((prev) => ({ ...prev, ...table }));
}

export function isRTL(code) {
  const rtl = ["ar", "he", "ur"];
  return rtl.some((l) => code.startsWith(l));
}
