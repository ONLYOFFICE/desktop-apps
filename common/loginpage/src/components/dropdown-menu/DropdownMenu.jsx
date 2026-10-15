import { Show, splitProps } from "solid-js";
import { DropdownMenu as DropdownMenuPrimitive } from "@kobalte/core/dropdown-menu";
import { mergeClass } from "@/utils/classes";
import { usePngIcons } from "@/services/display";
import styles from "../context-menu/ContextMenu.module.css";

function Root(props) {
  return <DropdownMenuPrimitive {...props} />;
}

function Trigger(props) {
  return <DropdownMenuPrimitive.Trigger {...props} />;
}

function Content(props) {
  const [, rest] = splitProps(props, ["class"]);
  return (
    <DropdownMenuPrimitive.Portal>
      <DropdownMenuPrimitive.Content classList={mergeClass(styles.menu, props.class)} {...rest} />
    </DropdownMenuPrimitive.Portal>
  );
}

function Item(props) {
  const [, rest] = splitProps(props, ["class", "icon", "children"]);
  return (
    <DropdownMenuPrimitive.Item classList={mergeClass(styles.menuItem, props.class)} {...rest}>
      <Show when={props.icon}>
        <Show when={usePngIcons()} fallback={<svg class={styles.menuIcon}><use href={props.icon} /></svg>}>
          <i class={`${styles.menuIcon} tool-icon__20 ${props.icon.slice(1)}`} />
        </Show>
      </Show>
      {props.children}
    </DropdownMenuPrimitive.Item>
  );
}

function Separator(props) {
  const [, rest] = splitProps(props, ["class"]);
  return <DropdownMenuPrimitive.Separator classList={mergeClass(styles.menuSeparator, props.class)} {...rest} />;
}

export const DropdownMenu = Object.assign(Root, { Trigger, Content, Item, Separator });
