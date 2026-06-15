import { splitProps } from "solid-js";
import { Dialog as DialogPrimitive } from "@kobalte/core/dialog";
import { mergeClass } from "@/utils/classes";
import styles from "./Dialog.module.css";

function Root(props) {
  return <DialogPrimitive {...props} />;
}

function Content(props) {
  const [local, rest] = splitProps(props, ["class"]);
  return (
    <DialogPrimitive.Portal>
      <DialogPrimitive.Overlay class={styles.overlay} />
      <DialogPrimitive.Content classList={mergeClass(styles.content, local.class)} {...rest} />
    </DialogPrimitive.Portal>
  );
}

function Title(props) {
  return <DialogPrimitive.Title {...props} />;
}

function CloseButton(props) {
  return <DialogPrimitive.CloseButton {...props} />;
}

export const Dialog = Object.assign(Root, { Content, Title, CloseButton });
