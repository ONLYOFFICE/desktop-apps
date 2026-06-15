import { Show, createEffect, lazy, onMount } from "solid-js";
import { HashRouter } from "@solidjs/router";
import Sidebar from "./shell/Sidebar";
import About from "./routes/about/index.jsx";
import { AiPanel, AiTogglePill } from "@/components/ai-panel";
import { signalAppReady } from "@/sdk";
import { initSettingsSync } from "@/services/settings-sync";
import { initAbout } from "@/services/about";
import { initAiPanel } from "@/services/ai-panel";
import { settings } from "@/stores/settings";
import { aichat } from "@/stores/aichat";
import "./styles/main.css";
import "./styles/layout.css";

function Shell(props) {
  createEffect(() => {
    const width = settings.useai && aichat.open ? `${aichat.width}px` : "0px";
    document.documentElement.style.setProperty("--ai-w", width);
  });

  return (
    <div class="shell" data-ai-open={aichat.open}>
      <div class="col-sidebar">
        <Sidebar />
      </div>
      <div class="col-center">
        {props.children}
        <Show when={settings.useai && !aichat.open}>
          <AiTogglePill />
        </Show>
      </div>
      <Show when={settings.useai && aichat.open}>
        <div class="col-ai">
          <AiPanel />
        </div>
      </Show>
      <About />
    </div>
  );
}

/** @type {import("@solidjs/router").RouteDefinition[]} */
const routes = [
  { path: "/", component: lazy(() => import("./routes/home/index.jsx")) },
  { path: "/open", component: lazy(() => import("./routes/open/index.jsx")) },
  { path: "/templates", component: lazy(() => import("./routes/templates/index.jsx")) },
  { path: "/clouds/*", component: lazy(() => import("./routes/clouds/index.jsx")) },
  {
    path: "/settings",
    component: lazy(() => import("./routes/settings/layout.jsx")),
    children: [{ path: "/general", component: lazy(() => import("./routes/settings/general/index.jsx")) }],
  },
];

export default function App() {
  initSettingsSync();
  initAbout();
  initAiPanel();
  onMount(signalAppReady);
  return <HashRouter root={Shell}>{routes}</HashRouter>;
}
