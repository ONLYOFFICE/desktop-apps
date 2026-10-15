import { createSignal } from 'solid-js';

const [message, setMessage] = createSignal(null);
let hideTimer;

export function showToast(text, duration = 2500) {
  clearTimeout(hideTimer);
  setMessage(text);
  hideTimer = setTimeout(() => setMessage(null), duration);
}

export { message };
