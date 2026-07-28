import { createSignal, createEffect, onMount, onCleanup, Switch, Match } from "solid-js";
import { IntroPanel } from "./components/IntroPanel";
import { StatusPanel } from "./components/StatusPanel";
import { resizeDialog, initAddonStatus, installAddon } from "./services/addon";
import { inputArgs } from "./services/url";

const PANEL_SIZES = {
  intro: { width: 480, height: 700 },
  default: { width: 480, height: 304 },
};

const [panel, setPanel] = createSignal("intro");

export function setViewPanel(p) {
  setPanel(p);

  const el = document.body;
  el.className = el.className.replace(/panel-[\w-]+/gi, "").trim();
  el.classList.add(`panel-${p}`);
}

export default function App() {
  const page = inputArgs()?.page;
  if (!!page)
    setViewPanel(page)

  createEffect(() => {
    const size = panel()=="intro" ? PANEL_SIZES.intro : PANEL_SIZES.default;
    resizeDialog(size.width, size.height);
  });

  onMount(() => {
    onCleanup(initAddonStatus());
  });

  function handleInstall() {
    installAddon();
    // setViewPanel("install");
  }

  return (
    <Switch>
      <Match when={panel() === "intro"}>
        <IntroPanel onInstall={handleInstall} />
      </Match>
      <Match when={["install", "complete", "error"].includes(panel())}>
        <StatusPanel page={panel()} />
      </Match>
    </Switch>
  );
}
