import { createSignal } from "solid-js";

const [inArgs, setInArgs] = createSignal({});

export function parseQueryParams() {
  const q = window.location.search.substring(1);
  const params = {};
  q.split("&").forEach((pair) => {
    const [k, v] = pair.split("=");
    if (k) params[decodeURIComponent(k)] = decodeURIComponent((v || "").replace(/\+/g, " "));
  });
  return params;
}

export function inputArgs() {
  if (!inArgs.filled) {
    setInArgs(parseQueryParams());
    inArgs.filled = true;
  }

  return inArgs();
}