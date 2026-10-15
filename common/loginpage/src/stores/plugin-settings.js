import { setIsOptsChanged } from "@/services/settings-sync";
import { createSignal } from "solid-js";
import { createStore } from "solid-js/store";

// [{ id: string, sections: Array<{ id: string, text: string, items?: SettingItem[], url?: string, disabled?: boolean }> }]
const [pluginConfigs, setPluginConfigs] = createSignal([]);

// flat map of current values keyed by "pluginId.sectionId.itemId"
const [pluginValues, setPluginValues] = createStore({});

export function registerPlugin(config) {
  if (config.type !== "settings" || !config.id || !config.items) return;

  const sections = config.items.map((section) => ({
    id: section.id,
    text: section.text,
    url: section.url,
    items: section.items ?? [],
  }));

  for (const section of sections) {
    for (const item of section.items) {
      setPluginValues(`${config.id}.${section.id}.${item.id}`, item.value ?? null);
    }
  }

  setPluginConfigs((prev) => {
    const idx = prev.findIndex((p) => p.id === config.id);
    const entry = { id: config.id, sections };
    if (idx >= 0) {
      const next = [...prev];
      next[idx] = entry;
      return next;
    }
    return [...prev, entry];
  });
}

export function getPluginValue(pluginId, sectionId, itemId) {
  return pluginValues[`${pluginId}.${sectionId}.${itemId}`];
}

export function setPluginValue(pluginId, sectionId, itemId, value) {
  setPluginValues(`${pluginId}.${sectionId}.${itemId}`, value);
  setIsOptsChanged(true);
}

export function getPluginSettings() {
  const configs = pluginConfigs();
  if (configs.length === 0) return undefined;

  const result = [];
  for (const plugin of configs) {
    for (const section of plugin.sections) {
      for (const item of section.items ?? []) {
        result.push({ id: item.id, value: pluginValues[`${plugin.id}.${section.id}.${item.id}`] });
      }
    }
  }
  return result;
}

export function selectPluginSection(sectionId) {
  for (const plugin of pluginConfigs()) {
    const section = plugin.sections.find((s) => s.id === sectionId);
    if (section) {
      window.location.hash = `#/settings/plugin/${encodeURIComponent(plugin.id)}/${encodeURIComponent(sectionId)}`;
      return;
    }
  }
  window.location.hash = "#/settings/general";
}

export function setPluginSectionsDisabled(ids, disabled) {
  const idSet = new Set(Array.isArray(ids) ? ids : [ids]);
  setPluginConfigs((prev) =>
    prev.map((plugin) => ({
      ...plugin,
      sections: plugin.sections.map((s) =>
        idSet.has(s.id) ? { ...s, disabled } : s
      ),
    }))
  );
}

export { pluginConfigs };
