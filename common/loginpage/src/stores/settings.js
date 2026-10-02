import { createStore } from "solid-js/store";
import { makePersisted } from "@solid-primitives/storage";

export const defs = {
  THEME_ID_DEFAULT_LIGHT: 'theme-white',
  THEME_ID_DEFAULT_DARK: 'theme-night',

  THEME_TYPE_LIGHT: 'light',
  THEME_TYPE_DARK:'dark',
  THEME_TYPE_SYSTEM: 'system',

  EDITOR_MODE_TAB:      'tab',
  EDITOR_MODE_WINDOW:   'window',
}

export const themesMap = {
        'theme-system': {
            // text: utils.Lang.settOptThemeSystem,
            type: defs.THEME_TYPE_SYSTEM,
        },
        'theme-light': {
            // text: utils.Lang.settOptThemeLight,
            type: 'light',
        },
        'theme-classic-light': {
            // text: utils.Lang.settOptThemeClassicLight,
            type: 'light',
        },
        'theme-dark': {
            // text: utils.Lang.settOptThemeDark,
            type: 'dark',
        },
        'theme-contrast-dark': {
            // text: utils.Lang.settOptThemeContrastDark,
            type: 'dark',
        },
        'theme-gray': {
            // text: utils.Lang.settOptThemeGray,
            type: 'light',
        },
        'theme-white': {
            // text: utils.Lang.settOptThemeWhite,
            type: 'light',
        },
        'theme-night': {
            // text: utils.Lang.settOptThemeNight,
            type: 'dark',
        },
    }

const defaultSettings = Object.freeze({
  userName: "",
  country: "US",
  lang: "en",
  theme: defs.THEME_ID_DEFAULT_LIGHT,
  launchMode: defs.EDITOR_MODE_TAB,
  spellCheck: "auto",
  gpuMode: true,
  useai: true,
  showai: false,
});

const [settings, setSettings] = makePersisted(createStore({ ...defaultSettings }), { name: "settings" });

export { settings, setSettings, defaultSettings };
