import { Show } from 'solid-js';
import styles from './Toast.module.css';
import { message } from './toastutil';

export default function Toast() {
  return (
    <Show when={message()}>
      <div class={styles.toast}>{message()}</div>
    </Show>
  );
}
