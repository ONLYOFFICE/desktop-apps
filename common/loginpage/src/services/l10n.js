import { createSignal } from "solid-js";
import { parseQueryParams } from "@/utils/url";

const l10n = {};
const localeModules = import.meta.glob("../../locale/*.js", { eager: true });
for (const path in localeModules) {
  const code = path.match(/([^/]+)\.js$/)[1];
  l10n[code.replaceAll("-", "_")] = localeModules[path].default;
}

const [translations, setTranslations] = createSignal({});

export function initLocale() {
  const en = l10n?.en ?? {};
  setTranslations({ ...en });

  const inParams = parseQueryParams();
  if (inParams.lang) applyLocale(inParams.lang);
}

export function t(key, defstring) {
  return translations()[key] ?? defstring ?? key;
}

export function applyLocale(locale) {
  const correct = locale.replaceAll("-", "_");
  let table = l10n?.[correct];

  if (!table) {
    const code = /^\w{2}/.exec(correct)?.[0];
    if (code) {
      for (const l in l10n || {}) {
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

export function getTranslationFor(locale, key) {
  const correct = locale.replaceAll("-", "_");
  let table = l10n?.[correct];
  if (!table) {
    const code = /^\w{2}/.exec(correct)?.[0];
    if (code) {
      for (const l in l10n || {}) {
        if (l.startsWith(code)) { table = l10n[l]; break; }
      }
    }
  }
  return table?.[key] ?? null;
}
