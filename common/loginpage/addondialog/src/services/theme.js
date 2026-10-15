import { createSignal, createEffect } from "solid-js";
import { onNativeMessage } from "./bridge";

// mirrors loginpage/src/stores/settings.js + services/about.js::applyThemeToDom,
// trimmed to just the theme resolution this dialog needs (no user settings store).
const THEME_ID_DEFAULT_LIGHT = "theme-white";
const THEME_ID_DEFAULT_DARK = "theme-night";

const THEME_TYPE_LIGHT = "light";
const THEME_TYPE_DARK = "dark";
const THEME_TYPE_SYSTEM = "system";

const themesMap = {
  "theme-system": { type: THEME_TYPE_SYSTEM },
  "theme-light": { type: "light" },
  "theme-classic-light": { type: "light" },
  "theme-dark": { type: "dark" },
  "theme-contrast-dark": { type: "dark" },
  "theme-gray": { type: "light" },
  "theme-white": { type: "light" },
  "theme-night": { type: "dark" },
};

function getDefaultThemeForType(type) {
  return type === THEME_TYPE_DARK ? THEME_ID_DEFAULT_DARK : THEME_ID_DEFAULT_LIGHT;
}

function getSystemThemeType() {
  const nativevars = window.RendererProcessVariable;
  if (nativevars?.theme && !!nativevars.theme.system) {
    return nativevars.theme.system !== "disabled" ? nativevars.theme.system : THEME_TYPE_LIGHT;
  }
  return window.matchMedia("(prefers-color-scheme: dark)").matches ? THEME_TYPE_DARK : THEME_TYPE_LIGHT;
}

function isSystemThemeDark() {
  return getSystemThemeType() === THEME_TYPE_DARK;
}

function relevantThemeId(id) {
  if (id === "theme-system") return getDefaultThemeForType(isSystemThemeDark() ? THEME_TYPE_DARK : THEME_TYPE_LIGHT);
  return id;
}

function applyThemeToDom(id) {
  if (!id) return;
  const themeId = relevantThemeId(id);
  const el = document.body;
  el.className = el.className.replace(/theme-[\w-]+/gi, "").trim();
  el.classList.add(themeId, `theme-type-${themesMap[themeId]?.type ?? "light"}`);
}

const [themeSetting, setThemeSetting] = createSignal(
  window.RendererProcessVariable?.theme?.id || THEME_ID_DEFAULT_LIGHT,
);

export function initTheme() {
  createEffect(() => applyThemeToDom(themeSetting()));

  function htmlDecode(str) {
    const ta = document.createElement("textarea");
    ta.innerHTML = str;
    return ta.value;
  }

  return onNativeMessage((cmd, param) => {
    if (cmd === "uitheme:changed") {
      setThemeSetting(param);
    } else if (cmd === "renderervars:changed") {
      let opts;
      try {
        opts = JSON.parse(htmlDecode(param));
      } catch {
        return;
      }
      if (opts?.theme?.system) {
        window.RendererProcessVariable.theme.system = opts.theme.system;
        applyThemeToDom(themeSetting());
      }
    }
  });
}
