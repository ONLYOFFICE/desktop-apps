import { createSignal } from "solid-js";

const [modals, setModals] = createSignal([]);

export function openPluginModal(id, url, width, height) {
  setModals((prev) => [...prev.filter((m) => m.id !== id), { id, url, width, height }]);
}

export function closePluginModal(id) {
  setModals((prev) => prev.filter((m) => m.id !== id));
}

export { modals };
