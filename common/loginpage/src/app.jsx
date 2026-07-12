import { Show, createEffect, lazy, onMount } from "solid-js";
import { HashRouter, useLocation } from "@solidjs/router";
import Sidebar from "./shell/Sidebar";
import About from "./routes/about/index.jsx";
import { AiPanel, ResizeHandle } from "@/components/ai-panel";
import { signalAppReady } from "@/sdk";
import { initSettingsSync } from "@/services/settings-sync";
import { initAbout } from "@/services/about";
import { initPortals } from "@/services/portals";
import { initAiPanel } from "@/services/ai-panel";
import { aichat } from "@/stores/aichat";
import "./styles/main.css";
import "./styles/layout.css";

function Shell(props) {
  const location = useLocation();
  const isSettings = () => location.pathname.startsWith("/settings");
  const aiActive = () => aichat.available && aichat.open && !isSettings();

  createEffect(() => {
    document.documentElement.style.setProperty("--ai-w", aiActive() ? `${aichat.width}px` : "0px");
  });

  return (
    <div class="shell" data-ai-open={aiActive()}>
      <div class="col-sidebar">
        <Sidebar />
      </div>
      <div class="col-center">
        {props.children}
        <Show when={aichat.available}>
          <div style={{ display: aichat.open && !isSettings() ? "contents" : "none" }}>
            <ResizeHandle />
          </div>
        </Show>
      </div>
      <Show when={aichat.available}>
        <div class="col-ai" style={{ display: aiActive() ? undefined : "none" }}>
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
    children: [
      { path: "/general", component: lazy(() => import("./routes/settings/general/index.jsx")) },
      { path: "/plugin/:pluginId/:section", component: lazy(() => import("./routes/settings/plugin/index.jsx")) },
    ],
  },
];

export default function App() {
  initSettingsSync();
  initAbout();
  initPortals();
  initAiPanel();
  onMount(signalAppReady);
  return <HashRouter root={Shell}>{routes}</HashRouter>;
}
