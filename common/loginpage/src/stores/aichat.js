import { createStore } from "solid-js/store";
import { makePersisted } from "@solid-primitives/storage";

export const AI_PANEL_DEFAULT_WIDTH = 400;
export const AI_PANEL_MIN_WIDTH = 300;
export const AI_CONTENT_MIN_WIDTH = 200;

const defaultAichat = {
  width: AI_PANEL_DEFAULT_WIDTH,
  url: "",
  available: false,
};

const persistedKeys = ["width"];

function hydrate(raw) {
  try {
    const saved = JSON.parse(raw);
    return {
      ...defaultAichat,
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

export function setChatAvialable(value) {
  setAichat("available", value);
}

export { aichat, setAichat, defaultAichat };
