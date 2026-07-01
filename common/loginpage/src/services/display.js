import { createSignal } from "solid-js";

const MQ = "screen and (min-resolution: 1.01dppx) and (max-resolution: 1.99dppx)";

const [usePngIcons, setUsePngIcons] = createSignal(false);

function getDprClass() {
  const dpr = window.devicePixelRatio || 1;
  if (dpr >= 1.75) return "pixel-ratio__1_75";
  if (dpr >= 1.5)  return "pixel-ratio__1_5";
  return "pixel-ratio__1_25";
}

export function initDisplay() {
  const mql = window.matchMedia(MQ);

  function update(matches) {
    setUsePngIcons(matches);
    document.body.classList.remove("pixel-ratio__1_25", "pixel-ratio__1_5", "pixel-ratio__1_75");
    if (matches) document.body.classList.add(getDprClass());
  }

  mql.addEventListener("change", e => update(e.matches));
  update(mql.matches);
}

export { usePngIcons };
