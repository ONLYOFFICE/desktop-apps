import { createSignal } from "solid-js";

const [extPlugins, setExtPlugins] = createSignal([]);

export { extPlugins };

export function addExtPlugin(plugin) {
  setExtPlugins((prev) => {
    if (prev.some((p) => p.id === plugin.id)) return prev;
    return [...prev, plugin];
  });
}

// Strip braces from native GUIDs for use in URL paths
export const pluginRouteId = (id) => id.replace(/[{}]/g, "").toLowerCase();
