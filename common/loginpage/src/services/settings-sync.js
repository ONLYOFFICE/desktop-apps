import { createEffect, createMemo, createSignal, onCleanup, batch } from "solid-js";
import { settings, setSettings, themesMap, defs as settDefs } from "@/stores/settings";
import { getPluginSettings } from "@/stores/plugin-settings";
import { onNativeMessage } from "@/sdk/events";
import { applyLocale, getTranslationFor, isRTL } from "@/services/l10n";
import { command } from "@/sdk/bridge";
import { htmlDecode } from "@/utils/html";
import { sendPluginSettings } from "@/services/ai-panel";
import { aichat } from "@/stores/aichat";
import { sendTemplatesSettings } from "@/routes/templates";


function injectCustomThemeCss(id, colors) {
  if (!id || !colors) return;
  const existing = document.querySelector(`style[data-theme-id="${id}"]`);
  if (existing) existing.remove();
  const vars = Object.entries(colors).map(([k, v]) => `--${k}: ${v};`).join(" ");
  const el = document.createElement("style");
  el.setAttribute("data-theme-id", id);
  el.textContent = `:root .${id} { ${vars} }`;
  document.head.appendChild(el);
}

    const get_default_theme_for_type = type => type == settDefs.THEME_TYPE_DARK ? settDefs.THEME_ID_DEFAULT_DARK : settDefs.THEME_ID_DEFAULT_LIGHT;

    const get_system_theme_type = () => {
        const nativevars = window.RendererProcessVariable;
        if ( nativevars.theme && !!nativevars.theme.system ) {
            return nativevars.theme.system !== 'disabled' ? nativevars.theme.system : settDefs.THEME_TYPE_LIGHT;
        } else {
            return window.matchMedia('(prefers-color-scheme: dark)').matches ? settDefs.THEME_TYPE_DARK : settDefs.THEME_TYPE_LIGHT;
        }
    }

    const is_system_theme_dark = function () {
        return get_system_theme_type() == 'dark';
    }

    const is_theme_system = function (id) {
        return id == 'theme-system'
    }

    const relevant_theme_id = function (id) {
        if ( is_theme_system(id) )
            return get_default_theme_for_type(is_system_theme_dark() ? settDefs.THEME_TYPE_DARK : settDefs.THEME_TYPE_LIGHT);
        return id;
    }

function applyThemeToDom(id) {
  if (id) {
    const theme_id = relevant_theme_id(id);

    const el = document.body;
    el.className = el.className.replace(/theme-[\w-]+/gi, "").trim();
    el.classList.add(theme_id, `theme-type-${!!themesMap[theme_id]?themesMap[theme_id].type:'light'}`);
  }
}

// function applyScaling(scaling) {
  // const html = document.body;
  // html.className = html.className.replace(/\bscale-\d+\b/g, "").trim();

  // if (scaling === "auto") {
  //   document.body.style.zoom = "";
  //   return;
  // }

  // const value = parseInt(scaling, 10);
  // if (isNaN(value)) return;

  // if (value !== 100) html.classList.add(`scale-${value}`);
  // document.body.style.zoom = value / 100;
// }

function checkScaling() {
    const matches = {
        'pixel-ratio__1': `screen (-webkit-max-device-pixel-ratio: 1.24)`,
        'pixel-ratio__1_25': `screen and (-webkit-min-device-pixel-ratio: 1.25) and (-webkit-max-device-pixel-ratio: 1.49),
                                screen and (min-resolution: 1.25dppx) and (max-resolution: 1.49dppx)`,
        'pixel-ratio__1_5': `screen and (-webkit-min-device-pixel-ratio: 1.5) and (-webkit-max-device-pixel-ratio: 1.74),
                                screen and (min-resolution: 1.5dppx) and (max-resolution: 1.74dppx)`,
        'pixel-ratio__1_75': `screen and (-webkit-min-device-pixel-ratio: 1.75) and (-webkit-max-device-pixel-ratio: 1.99),
                                screen and (min-resolution: 1.75dppx) and (max-resolution: 1.99dppx)`,
        'pixel-ratio__2': `screen and (-webkit-min-device-pixel-ratio: 2) and (-webkit-max-device-pixel-ratio: 2.24),
                                screen and (min-resolution: 2dppx) and (max-resolution: 2.24dppx),
                                screen and (min-resolution: 192dpi) and (max-resolution: 215dpi)`,
        'pixel-ratio__2_5': `screen and (-webkit-min-device-pixel-ratio: 2.25), screen and (min-resolution: 2.25dppx),
                                screen and (min-resolution: 216dpi)`,
    };

    for (var c in matches) {
        const media_query_list = window.matchMedia(matches[c]);
        media_query_list.addEventListener('change', function(cls, e) {
            if ( e.matches ) {
                document.body.className = document.body.className.replace(/pixel-ratio__[\d_]+/gi, '').trim();
                document.body.classList.add(cls);
            }
        }.bind(this, c));

        if ( media_query_list.matches ) {
            document.body.classList.add(c);
            // break;
        }
    }
}

function nativeScalingToStore(raw) {
  if (raw == null) return "auto";

  const value = String(raw).trim();
  return value === "" || value === "0" ? "auto" : value;
}

const [availableLangs, setAvailableLangs] = createSignal([]);
const [langRestartPending, setLangRestartPending] = createSignal(false);
const [customThemes, setCustomThemes] = createSignal([]);
const [systemThemeEnabled, setSystemThemeEnabled] = createSignal(true);
const [isOptsChanged, setIsOptsChanged] = createSignal(false);
const [pendingApplyText, setPendingApplyText] = createSignal(null);
const [userNameError, setUserNameError] = createSignal(false);
const [appliedUseAi, setAppliedUseAi] = createSignal(settings.useai);
export { availableLangs, langRestartPending, customThemes, systemThemeEnabled, isOptsChanged, setIsOptsChanged, pendingApplyText, userNameError, setUserNameError, appliedUseAi };

let _applySettings = () => {};
let _removeAddon = () => {};
let _textDirectionChanged = () => {};

export const applySettings = () => _applySettings();
export const uninstallAddon = () => _removeAddon();
export const textDirectionChanged = () => _textDirectionChanged();

export function initSettingsSync() {
  const [nativeSnapshot, setNativeSnapshot] = createSignal(null);
  let initialized = false;

  checkScaling();

  const country = localStorage.getItem("sett-country") || "US";
  setSettings("country", country);

  const storedUsername = localStorage.getItem("username");
  if (storedUsername) {
    setSettings("userName", storedUsername);
  } else {
    command("settings:get", "username");
  }

  const nativevars = window.RendererProcessVariable;
  if (nativevars?.theme?.system === "disabled") setSystemThemeEnabled(false);
  if (nativevars?.localthemes?.length) {
    const loaded = nativevars.localthemes.flatMap((t) => {
      if (!t?.id) return [];
      injectCustomThemeCss(t.id, t.colors);
      return [{ value: t.id, label: t.name || t.id }];
    });
    if (loaded.length) setCustomThemes(loaded);
  }
  applyThemeToDom(settings.theme);


  _textDirectionChanged = createMemo(() => {
    const snap = nativeSnapshot();
    if (!snap) return false;

    return isRTL(settings.lang) !== isRTL(snap.locale?.current ?? settings.lang);
  })


  _removeAddon = () => {
    command("addon:action", "uninstall");
  };

  createEffect(() => setPendingApplyText(getTranslationFor(settings.lang, 'setBtnApply')));
  // createEffect(() => applyScaling(settings.scaling));

  createEffect(() => {
    settings.lang; settings.scaling; settings.theme;
    settings.checkUpdates; settings.launchMode; settings.spellCheck;
    settings.gpuMode; settings.userName; settings.useai;
    settings.country;
    if (!initialized) return;
    setIsOptsChanged(true);
  });

  const _validate_user_name = name => {
    if (!name || !name.length)
      return false;

      // return /^[\p{L}\p{M}\p{N}'"\.\- ]+$/u.test(name);

      /*  @winxpsupport
       *  we use chrome ver 49 on win xp,
       *  that version has no support for unicode in regexp
       */
      if (/*window.utils.inParams.osver == 'winxp' ||*/ /windows nt 5/i.test(navigator.appVersion)) return true;
      else return (new RegExp('^[\\p{L}\\p{M}\\p{N}\'"«»()_+=&^%$#@!~*\\/.\\- ]+$', 'iu')).test(name)
    };

  _applySettings = () => {
    if (!_validate_user_name(settings.userName)) {
      setUserNameError(true);
      return;
    }
    setUserNameError(false);

    const snap = nativeSnapshot();
    const lang = settings.lang;
    const scaling = settings.scaling;
    const updates = settings.checkUpdates;
    const launch = settings.launchMode;
    const spell = settings.spellCheck;

    let restart = false;
    if (snap !== null) {
      restart = (aichat.available && settings.useai !== !!snap.useai) ||
                  scaling !== nativeScalingToStore(snap.uiscaling) ||
                  settings.gpuMode !== !!snap.usegpu ||
                  textDirectionChanged() || langRestartPending();
    }

    snap.useai = settings.useai;
    snap.usegpu = settings.gpuMode;

    !langRestartPending() && !_textDirectionChanged() && applyLocale(lang);

    command("settings:apply", JSON.stringify({
      username: settings.userName,
      langid: settings.lang,
      uitheme: settings.theme,
      autoupdatemode: updates,
      editorwindowmode: launch == settDefs.EDITOR_MODE_WINDOW,
      spellcheckdetect: spell,
      usegpu: settings.gpuMode,
      useai: settings.useai,
      restart: restart,
      uiscaling: scaling === "auto" ? "0" : scaling,
    }));

    localStorage.setItem("sett-country", settings.country);
    sendTemplatesSettings({country: settings.country});

    const _plugin_opts = getPluginSettings();
    if (!!_plugin_opts) {
      console.log("plugin settings", _plugin_opts);
      sendPluginSettings(_plugin_opts);
    }

    setAppliedUseAi(settings.useai);
    setIsOptsChanged(false);
  };

  const unsub = onNativeMessage((msgCmd, param) => {
    if (msgCmd === "settings:init") {
      let payload;
      try {
        payload = JSON.parse(htmlDecode(param));
      } catch {
        return;
      }

      if (payload.locale?.langs) {
        const langs = Object.entries(payload.locale.langs).map(([code, info]) => ({
          code,
          name: info.name,
          enname: info.enname,
        }));
        if (langs.length > 0) setAvailableLangs(langs);
      }

      batch(() => {
        if (payload.locale?.current) setSettings("lang", payload.locale.current);
        if ("uiscaling" in payload)
          setSettings("scaling", nativeScalingToStore(payload.uiscaling));
        else setSettings("scalinglocked", true);
        if (payload.uitheme) {
          setSettings("theme", payload.uitheme);
          applyThemeToDom(payload.uitheme);
        }
        if (payload.autoupdatemode) setSettings("checkUpdates", payload.autoupdatemode);
        if (payload.editorwindowmode) setSettings("launchMode", payload.editorwindowmode ? settDefs.EDITOR_MODE_WINDOW : settDefs.EDITOR_MODE_TAB);
        if (payload.spellcheckdetect) setSettings("spellCheck", payload.spellcheckdetect);
        if (payload.usegpu != null) setSettings("gpuMode", !!payload.usegpu);
        if (payload.useai != null) {
          setSettings("useai", !!payload.useai);
          setAppliedUseAi(!!payload.useai);
        }
      });
      if (payload.rtl === true) {
        document.body.setAttribute("dir", "rtl");
        document.body.classList.add("rtl");
      }
      if (payload.locale?.current) {
        if (isRTL(payload.locale.current)) document.body.classList.add("rtl-font");
        document.body.setAttribute("applang", payload.locale.current);
      }

      if (payload.username) setSettings("userName", payload.username);
      if (payload.locale?.restart) setLangRestartPending(true);

      initialized = true;
      setNativeSnapshot(payload);
    } else if (msgCmd === "settings:username") {
      if (param && param.length) {
        setSettings("userName", param);
        setUserNameError(false);
      }
    } else if (msgCmd === "settings:lang") {
      if (param.startsWith("restart:")) setLangRestartPending(param === "restart:true");
    } else if (msgCmd === "addon:action") {
      if (param == "disable") {
        const state = isOptsChanged();

        setSettings("useai", false);
        setAppliedUseAi(false);

        !state && setIsOptsChanged(false);
      }
    } else if (msgCmd === "uitheme:added") {
      let themes;
      try {
        themes = JSON.parse(param);
      } catch {
        return;
      }
      if (!themes) return;
      const arr = Array.isArray(themes) ? themes : [themes];
      const added = [];
      for (const t of arr) {
        if (!t?.id) continue;
        injectCustomThemeCss(t.id, t.colors);
        added.push({ value: t.id, label: t.name || t.id });
      }
      if (added.length) {
        setCustomThemes((prev) => {
          const next = prev.filter((x) => !added.some((a) => a.value === x.value));
          return [...next, ...added];
        });
        setSettings("theme", added[0].value);
      }
    } else if (msgCmd === "renderervars:changed") {
      let opts;
      try {
        opts = JSON.parse(htmlDecode(param));
      } catch {
        return;
      }
      if (opts?.theme?.system) {
        window.RendererProcessVariable.theme.system = opts.theme.system;
      }
    } else if (msgCmd === "uitheme:changed") {
      setSettings("theme", param);
      applyThemeToDom(settings.theme);
    }
  });

  onCleanup(unsub);
}
