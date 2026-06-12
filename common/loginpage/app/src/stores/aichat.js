import { createStore } from "solid-js/store";
import { makePersisted } from "@solid-primitives/storage";

export const AI_PANEL_DEFAULT_WIDTH = 400;
export const AI_PANEL_MIN_WIDTH = 400;
export const AI_CONTENT_MIN_WIDTH = 800;

const defaultAichat = {
  open: true,
  width: AI_PANEL_DEFAULT_WIDTH,
  url: "",
};

const persistedKeys = ["open", "width"];

function hydrate(raw) {
  try {
    const saved = JSON.parse(raw);
    return {
      ...defaultAichat,
      open: typeof saved.open === "boolean" ? saved.open : defaultAichat.open,
      width: Number.isFinite(saved.width) ? saved.width : defaultAichat.width,
    };
  } catch {
    return { ...defaultAichat };
  }
}

const [aichat, setAichat] = makePersisted(createStore({ ...defaultAichat }), {
  name: "aichat",
  serialize: (state) => JSON.stringify(Object.fromEntries(persistedKeys.map((k) => [k, state[k]]))),
  deserialize: hydrate,
});

export function setPanelUrl(url) {
  setAichat("url", url ?? "");
}

export function collapsePanel() {
  setAichat("open", false);
}

export function expandPanel() {
  setAichat("open", true);
}

export { aichat, setAichat, defaultAichat };
