import { For, Show } from "solid-js";
import { useParams } from "@solidjs/router";
import { Select, Switch, TextField } from "@/components";
import { pluginConfigs, getPluginValue, setPluginValue } from "@/stores/plugin-settings";
import styles from "./index.module.css";

export default function PluginSettings() {
  const params = useParams();
  const pluginId = () => decodeURIComponent(params.pluginId);
  const sectionName = () => decodeURIComponent(params.section);

  const sectionValue = () => {
    const plugin = pluginConfigs().find((p) => p.id === pluginId());
    return plugin?.sections[sectionName()] ?? [];
  };

  const isUrl = () => typeof sectionValue() === "string";

  return (
    <Show when={!isUrl()}>
      <div class={styles.page}>
        <div class={styles.form}>
          <For each={sectionValue()}>
            {(item) => {
              const value = () => getPluginValue(pluginId(), sectionName(), item.id);
              const onChange = (v) => setPluginValue(pluginId(), sectionName(), item.id, v);

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
