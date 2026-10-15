import { For, Show } from "solid-js";
import { useParams } from "@solidjs/router";
import { Select, Switch, TextField } from "@/components";
import { pluginConfigs, getPluginValue, setPluginValue } from "@/stores/plugin-settings";
import styles from "./index.module.css";

export default function PluginSettings() {
  const params = useParams();
  const pluginId = () => decodeURIComponent(params.pluginId);
  const sectionId = () => decodeURIComponent(params.section);

  const currentSection = () => {
    const plugin = pluginConfigs().find((p) => p.id === pluginId());
    return plugin?.sections.find((s) => s.id === sectionId());
  };

  const sectionItems = () => currentSection()?.items ?? [];
  const isUrl = () => !!currentSection()?.url;

  return (
    <Show when={!isUrl()}>
      <div class={styles.page}>
        <div class={styles.form}>
          <For each={sectionItems()}>
            {(item) => {
              const value = () => getPluginValue(pluginId(), sectionId(), item.id);
              const onChange = (v) => setPluginValue(pluginId(), sectionId(), item.id, v);

              if (item.type === "combobox") {
                return (
                  <div class={styles.fieldRow}>
                    <span class={styles.fieldLabel}>{item.text}</span>
                    <div class={styles.fieldControl}>
                      <Select
                        options={(item.items ?? []).map((v) => ({ value: v, label: v }))}
                        value={value()}
                        onChange={onChange}
                        disallowEmptySelection
                      >
                        <Select.Trigger />
                        <Select.Content />
                      </Select>
                    </div>
                  </div>
                );
              }

              if (item.type === "checkbox") {
                return (
                  <div class={styles.fieldRowAuto}>
                    <label for={`plugin-item-${item.id}`} class={styles.fieldLabelBlock}>
                      {item.text}
                    </label>
                    <Switch id={`plugin-item-${item.id}`} checked={!!value()} onChange={onChange} />
                  </div>
                );
              }

              if (item.type === "text") {
                return (
                  <div class={styles.fieldRow}>
                    <span class={styles.fieldLabel}>{item.text}</span>
                    <div class={styles.fieldControl}>
                      <TextField value={String(value() ?? "")} onChange={onChange}>
                        <TextField.Input />
                      </TextField>
                    </div>
                  </div>
                );
              }

              return null;
            }}
          </For>
        </div>
      </div>
    </Show>
  );
}
