import { Match, Show, Switch, createSignal, splitProps } from "solid-js";
import { A } from "@solidjs/router";
import { usePngIcons } from "@/services/display";

export function MenuItem(props) {
  const [local] = splitProps(props, ["href", "onClick", "icon", "label", "end", "hidden", "action", "class", "isActive"]);

  const [pngvisible, setPngVisible] = createSignal();

  return (
    <li class="menu-item" classList={{ hidden: local.hidden }}>
      <Switch>
        <Match when={local.href}>
          <A href={local.href} onClick={local.onClick} activeClass="selected" end={local.end} action={local.action} class={local.class} classList={{ selected: local.isActive?.() }}>
            <Show when={local.icon}>
              <div class="icon-inner-box">
                <Show when={usePngIcons()} fallback={<svg class="icon"><use href={local.icon} /></svg>}>
                  <i class={`icon tool-icon ${local.icon.slice(1)}`} />
                </Show>
              </div>
            </Show>
            <span class="text">{local.label}</span>
          </A>
        </Match>
        <Match when={local.onClick}>
          <button class="menu-btn" onClick={local.onClick} class={local.class}>
            <Show when={local.icon}>
              <div class="icon-inner-box">
                <Show when={usePngIcons()} fallback={<svg class="icon"><use href={local.icon} /></svg>}>
                  <i class={`icon tool-icon ${local.icon.slice(1)}`} />
                </Show>
              </div>
            </Show>
            <span class="text">{local.label}</span>
          </button>
        </Match>
      </Switch>
    </li>
  );
}
