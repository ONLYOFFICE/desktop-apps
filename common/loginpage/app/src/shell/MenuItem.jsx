import { Match, Switch, splitProps } from "solid-js";
import { A } from "@solidjs/router";

export function MenuItem(props) {
  const [local] = splitProps(props, ["href", "onClick", "icon", "label", "end", "hidden"]);

  return (
    <li class="menu-item" classList={{ hidden: local.hidden }}>
      <Switch>
        <Match when={local.href}>
          <A href={local.href} activeClass="selected" end={local.end}>
            <svg class="icon">
              <use href={local.icon} />
            </svg>
            <span class="text">{local.label}</span>
          </A>
        </Match>
        <Match when={local.onClick}>
          <button class="menu-btn" onClick={local.onClick}>
            <svg class="icon">
              <use href={local.icon} />
            </svg>
            <span class="text">{local.label}</span>
          </button>
        </Match>
      </Switch>
    </li>
  );
}
