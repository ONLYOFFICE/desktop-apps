import { setIsOptsChanged } from "@/services/settings-sync";
import { createSignal } from "solid-js";
import { createStore } from "solid-js/store";

// [{ id: string, sections: { [sectionName]: SettingItem[] } }]
const [pluginConfigs, setPluginConfigs] = createSignal([]);

// flat map of current values keyed by "pluginId.section.itemId"
const [pluginValues, setPluginValues] = createStore({});

export function registerPlugin(config) {
  if (config.type !== "settings" || !config.id || !config.items) return;

  for (const [section, items] of Object.entries(config.items)) {
    for (const item of items) {
      setPluginValues(`${config.id}.${section}.${item.id}`, item.value ?? null);
    }
  }

  setPluginConfigs((prev) => {
    const idx = prev.findIndex((p) => p.id === config.id);
    const entry = { id: config.id, sections: config.items };
    if (idx >= 0) {
      const next = [...prev];
      next[idx] = entry;
      return next;
    }
    return [...prev, entry];
  });
}

export function getPluginValue(pluginId, section, itemId) {
  return pluginValues[`${pluginId}.${section}.${itemId}`];
}

export function setPluginValue(pluginId, section, itemId, value) {
  setPluginValues(`${pluginId}.${section}.${itemId}`, value);
  setIsOptsChanged(true);
}

export function getPluginSettings() {
  const configs = pluginConfigs();
  if (configs.length === 0) {
    return undefined;
  }

  const result = [];
  for (const plugin of configs) {
    for (const [section, items] of Object.entries(plugin.sections)) {
      for (const item of items) {
        result.push({ id: item.id, value: pluginValues[`${plugin.id}.${section}.${item.id}`] });
      }
    }
  }
  console.log(result);
  return result;
}

export { pluginConfigs };
