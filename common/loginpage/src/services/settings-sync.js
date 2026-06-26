import { createEffect, createMemo, createSignal, onCleanup, batch } from "solid-js";
import { settings, setSettings, themesMap } from "@/stores/settings";
import { getPluginSettings } from "@/stores/plugin-settings";
import { onNativeMessage } from "@/sdk/events";
import { cmd } from "@/sdk/bridge";
import { applyLocale } from "@/services/l10n";
import { htmlDecode } from "@/utils/html";
import { sendPluginSettings } from "@/sdk/app";


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

function applyThemeToDom(id) {
  const el = document.body;
  el.className = el.className.replace(/theme-[\w-]+/gi, "").trim();
  if (id) {
    el.classList.add(id, `theme-type-${!!themesMap[id]?themesMap[id].type:'light'}`);
  }
}

function applyScaling(scaling) {
  const html = document.body;
  html.className = html.className.replace(/\bscale-\d+\b/g, "").trim();

  if (scaling === "auto") {
    document.body.style.zoom = "";
    return;
  }

  const value = parseInt(scaling, 10);
  if (isNaN(value)) return;

  if (value !== 100) html.classList.add(`scale-${value}`);
  document.body.style.zoom = value / 100;
}

function nativeScalingToStore(raw) {
  if (raw == null) return "auto";

  const value = String(raw).trim();
  return value === "" || value === "0" ? "auto" : value;
}

const [availableLangs, setAvailableLangs] = createSignal(FALLBACK_LANGS);
const [langRestartPending, setLangRestartPending] = createSignal(false);
const [customThemes, setCustomThemes] = createSignal([]);
const [systemThemeEnabled, setSystemThemeEnabled] = createSignal(true);
const [isOptsChanged, setIsOptsChanged] = createSignal(false);
export { availableLangs, langRestartPending, customThemes, systemThemeEnabled, isOptsChanged, setIsOptsChanged };

let _needsRestart = () => false;
let _toggleAI = () => {};
let _applySettings = () => {};

export const needsRestart = () => _needsRestart();
export const toggleAI = () => _toggleAI();
export const applySettings = () => _applySettings();

export function initSettingsSync() {
  const [nativeSnapshot, setNativeSnapshot] = createSignal(null);
  let initialized = false;

  const storedUsername = localStorage.getItem("username");
  if (storedUsername) {
    setSettings("userName", storedUsername);
  } else {
    cmd("settings:get", "username");
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

  _needsRestart = createMemo(() => {
    const snap = nativeSnapshot();
    if (!snap) return false;
    return (
      settings.lang !== (snap.locale?.current ?? settings.lang) ||
      settings.scaling !== nativeScalingToStore(snap.uiscaling) ||
      settings.gpuMode !== !!snap.usegpu
    );
  });

  _toggleAI = () => {
    const newValue = !settings.useai;
    setSettings("useai", newValue);
    cmd("settings:apply", JSON.stringify({ useai: newValue }));
  };

  createEffect(() => applyLocale(settings.lang));
  createEffect(() => applyScaling(settings.scaling));

  createEffect(() => {
    settings.lang; settings.scaling; settings.theme;
    settings.checkUpdates; settings.launchMode; settings.spellCheck;
    settings.gpuMode; settings.promoButtons; settings.userName;
    if (!initialized) return;
    setIsOptsChanged(true);
  });

  _applySettings = () => {
    const snap = nativeSnapshot();
    const lang = settings.lang;
    const scaling = settings.scaling;
    // const theme = settings.theme;
    const updates = settings.checkUpdates;
    const launch = settings.launchMode;
    const spell = settings.spellCheck;
    const gpu = settings.gpuMode;

    const restart =
      snap !== null &&
      (lang !== (snap.locale?.current ?? lang) || scaling !== nativeScalingToStore(snap.uiscaling) || gpu !== !!snap.usegpu);
    applyThemeToDom(typeof settings.theme === 'string' ? settings.theme : '');

    cmd("settings:apply", JSON.stringify({
      username: settings.userName,
      lang,
      uitheme: settings.theme,
      autoupdatemode: updates,
      docopenmode: launch,
      spellcheckdetect: spell,
      usegpu: gpu,
      restart: _needsRestart(),
      uiscaling: scaling === "auto" ? "0" : scaling,
    }));

    const _plugin_opts = getPluginSettings();
    if (!!_plugin_opts) {
      console.log("plugin settings", _plugin_opts);
      sendPluginSettings(_plugin_opts);
    }

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
        if ("uiscaling" in payload) setSettings("scaling", nativeScalingToStore(payload.uiscaling));
        if (payload.uitheme) setSettings("theme", payload.uitheme);
        if (payload.autoupdatemode) setSettings("checkUpdates", payload.autoupdatemode);
        if (payload.docopenmode) setSettings("launchMode", payload.docopenmode);
        if (payload.spellcheckdetect) setSettings("spellCheck", payload.spellcheckdetect);
        if (payload.usegpu != null) setSettings("gpuMode", !!payload.usegpu);
        if (payload.useai != null) setSettings("useai", !!payload.useai);
      });
      if (payload.rtl === true) {
        document.body.setAttribute("dir", "rtl");
        document.body.classList.add("rtl");
      }
      if (payload.username) setSettings("userName", payload.username);
      if (payload.locale?.restart) setLangRestartPending(true);

      initialized = true;
      setNativeSnapshot(payload);
    } else if (msgCmd === "settings:username") {
      if (param) setSettings("userName", param);
    } else if (msgCmd === "settings:lang") {
      if (param.startsWith("restart:")) setLangRestartPending(param === "restart:true");
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
      if (opts?.theme?.system) setSystemThemeEnabled(opts.theme.system !== "disabled");
    } else if (msgCmd === "uitheme:changed") {
      setSettings("theme", param);
    }
  });

  onCleanup(unsub);
}
