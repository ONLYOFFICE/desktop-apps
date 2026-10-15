import { For } from "solid-js";
import { Dialog } from "@/components/dialog/Dialog";
import { modals, closePluginModal } from "@/stores/plugin-modals";
import styles from "./PluginModal.module.css";

export function PluginModal() {
  return (
    <For each={modals()}>
      {(modal) => (
        <Dialog open onOpenChange={(open) => !open && closePluginModal(modal.id)}>
          <Dialog.Content
            class={styles.content}
            style={{
              width: modal.width ? `${modal.width}px` : "80vw",
              height: modal.height ? `${modal.height}px` : "80vh",
            }}
          >
            <iframe src={modal.url} class={styles.frame} title={modal.id} />
          </Dialog.Content>
        </Dialog>
      )}
    </For>
  );
}
