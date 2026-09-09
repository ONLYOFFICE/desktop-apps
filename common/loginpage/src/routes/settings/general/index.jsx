import { createMemo, batch, Show } from "solid-js";
import { t } from "@/services/l10n";
import { Select, Switch, TextField, Button } from "@/components";
import { settings, setSettings, defaultSettings } from "@/stores/settings";
import { availableLangs,
        systemThemeEnabled,
        userNameError,
        setUserNameError,
        uninstallAddon,
        textDirectionChanged,
        setIsOptsChanged,
        langRestartPending} from "@/services/settings-sync";
import { command } from "@/sdk/bridge";
import styles from "./index.module.css";
import { addonInfo } from "@/services/about";
import { aichat } from "@/stores/aichat";
import { TEMPLATES_COUNTRY_ARRAY } from "@/data/countries";

export default function GeneralSettings() {
  const countryOptions = createMemo(() => TEMPLATES_COUNTRY_ARRAY);

  const languageOptions = createMemo(() => availableLangs().map((l) => ({
    value: l.code,
    label: l.name,
    sublabel: l.enname,
  })));

  const scalingOptions = createMemo(() => [
    { value: "auto", label: t("settOptScalingAuto") },
    { value: "100", label: "100%" },
    { value: "125", label: "125%" },
    { value: "150", label: "150%" },
    { value: "175", label: "175%" },
    { value: "200", label: "200%" },
    { value: "225", label: "225%" },
    { value: "250", label: "250%" },
    { value: "275", label: "275%" },
    { value: "300", label: "300%" },
    { value: "350", label: "350%" },
    { value: "400", label: "400%" },
    { value: "450", label: "450%" },
    { value: "500", label: "500%" },
  ]);

  const updatesOptions = createMemo(() => [
    { value: "silent", label: t("settOptAUpdateSilent"), sublabel: t("settOptDescAUpdateSilent") },
    { value: "ask", label: t("settOptAUpdateAsk"), sublabel: t("settOptDescAUpdateAsk") },
    { value: "disabled", label: t("settOptAUpdateDisabled"), sublabel: t("settOptDescDisabled") },
  ]);

  const themeOptions = createMemo(() => [
    systemThemeEnabled() &&
    { value: "theme-system", label: t("settOptThemeSystem") },
    { value: "theme-white", label: t("settOptThemeWhite") },
    { value: "theme-night", label: t("settOptThemeNight") },
    { value: "theme-light", label: t("settOptThemeLight") },
    { value: "theme-classic-light", label: t("settOptThemeClassicLight") },
    { value: "theme-dark", label: t("settOptThemeDark") },
    { value: "theme-contrast-dark", label: t("settOptThemeContrastDark") },
    { value: "theme-gray", label: t("settOptThemeGray") },
  ].filter(Boolean));

  const openFileOptions = createMemo(() => [
    { value: "tab", label: t("settOptLaunchInTab") },
    { value: "window", label: t("settOptLaunchInWindow") },
  ]);

  const spellOptions = createMemo(() => [
    { value: "auto", label: t("settOptScalingAuto") },
    { value: "enabled", label: t("settOptEnabled") },
    { value: "disabled", label: t("settOptDisabled") },
  ]);

  function handleReset() {
    command("settings:get", "username");
  }


  return (
    <div class={styles.page}>
      <div class={styles.form}>
        <div class={styles.formHeader}>
          <button class={styles.resetLink} onClick={handleReset}>
            {t("settResetUserName")}
          </button>
        </div>

        <div class={styles.fieldRow}>
          <span class={styles.fieldLabel}>{t("settUserName")}</span>
          <div class={styles.fieldControl}>
            <TextField
              class={styles.fullWidth}
              value={settings.userName}
              validationState={userNameError() ? "invalid" : "valid"}
              onChange={(v) => { setUserNameError(false); setSettings("userName", v); }}
            >
              <TextField.Input placeholder={t("settingsUserNamePlaceholder")} />
            </TextField>
          </div>
        </div>

        <Show when={0 && countryOptions().length}>
          <div class={styles.fieldRow}>
            <span class={styles.fieldLabel}>{t("settingsCountry")}</span>
            <div class={styles.fieldControl}>
              <Select options={countryOptions()} value={settings.country} onChange={(v) => setSettings("country", v)} disallowEmptySelection>
                <Select.Trigger class={styles.fullWidth} />
                <Select.Content />
              </Select>
            </div>
          </div>
        </Show>

        <Show when={languageOptions().length}>
          <div class={styles.fieldRow}>
            <span class={styles.fieldLabel}>{t("settLanguage")}{(textDirectionChanged() || langRestartPending()) && ' *'}</span>
            <div class={styles.fieldControl}>
              <Select options={languageOptions()} value={settings.lang} onChange={(v) => setSettings("lang", v)} disallowEmptySelection>
                <Select.Trigger class={styles.fullWidth} />
                <Select.Content />
              </Select>
            </div>
          </div>
        </Show>

        <Show when={settings.scaling}>
          <div class={styles.fieldRow}>
            <span class={styles.fieldLabel}>{t("settScaling")} *</span>
            <div class={styles.fieldControl}>
              <Select options={scalingOptions()} value={settings.scaling} onChange={(v) => setSettings("scaling", v)} disallowEmptySelection>
                <Select.Trigger class={styles.fullWidth} />
                <Select.Content />
              </Select>
            </div>
          </div>
        </Show>

        <Show when={!!settings.checkUpdates}>
          <div class={styles.fieldRow}>
            <span class={styles.fieldLabel}>{t("settAUpdateMode")}</span>
            <div class={styles.fieldControl}>
              <Select
                options={updatesOptions()}
                value={settings.checkUpdates}
                onChange={(v) => setSettings("checkUpdates", v)}
                disallowEmptySelection
                stacked
              >
                <Select.Trigger class={styles.fullWidth} />
                <Select.Content />
              </Select>
            </div>
          </div>
        </Show>

        <div class={styles.fieldRow}>
          <span class={styles.fieldLabel}>{t("settUITheme")}</span>
          <div class={styles.fieldControl}>
            <Select options={themeOptions()} value={settings.theme} onChange={(v) => setSettings("theme", v)} disallowEmptySelection>
              <Select.Trigger class={styles.fullWidth} />
              <Select.Content />
            </Select>
          </div>
        </div>

        <div class={styles.fieldRow}>
          <span class={styles.fieldLabel}>{t("settOptLaunchMode")}</span>
          <div class={styles.fieldControl}>
            <Select
              options={openFileOptions()}
              value={settings.launchMode}
              onChange={(v) => setSettings("launchMode", v)}
              disallowEmptySelection
            >
              <Select.Trigger class={styles.fullWidth} />
              <Select.Content />
            </Select>
          </div>
        </div>

        <div class={styles.fieldRow}>
          <span class={styles.fieldLabel}>{t("settSpellcheckDetection")}</span>
          <div class={styles.fieldControl}>
            <Select
              options={spellOptions()}
              value={settings.spellCheck}
              onChange={(v) => setSettings("spellCheck", v)}
              disallowEmptySelection
            >
              <Select.Trigger class={styles.fullWidth} />
              <Select.Content />
            </Select>
          </div>
        </div>

        <div class={styles.fieldRowAuto}>
          <label for="gpu-mode" class={styles.fieldLabelBlock}>
            {t("settGpuUseMode")} *<span class={styles.fieldDescription}>{t("settingsGpuUseModeDescr")}</span>
          </label>
          <Switch id="gpu-mode" checked={settings.gpuMode} onChange={(v) => setSettings("gpuMode", v)} />
        </div>

        <div class={styles.fieldRowAuto}>
          <label for="chb-use-ai" class={styles.fieldLabelBlock}>
            {t("settingsEnableAI")}{aichat.available && ' *'}<span class={styles.fieldDescription}>{t("settingsEnableAIDescr")}</span>
          </label>
          <Switch id="chb-use-ai" checked={settings.useai !== false} onChange={(v) => setSettings("useai", v)} />
        </div>

        <Show when={addonInfo()?.installed}>
          <div class={styles.divider} />

          <div class={styles.fieldRowAuto}>
            <div class={styles.fieldLabelBlock}>
              <span class={styles.fieldLabel}>{t("settRemoveAddon")} *</span>
              <span class={styles.fieldDescription}>{t("descrRemoveAddon")}</span>

              <Show when={addonInfo().license?.url}>
                <p class="about-field about-license" style={{margin:0}}>
                  {t("strLicensedUnder")}{" "}
                  <a class="about-link" href={addonInfo().license.url} target="_blank" rel="noreferrer">
                    {addonInfo().license.title ?? addonInfo().license.url}
                  </a>
                </p>
              </Show>
            </div>
            <div class={styles.fieldControlEnd}>
              <Button variant="accent" onClick={uninstallAddon}>
                {t("btnUninstAddon")}
              </Button>
            </div>
          </div>
        </Show>
      </div>
    </div>
  );
}
