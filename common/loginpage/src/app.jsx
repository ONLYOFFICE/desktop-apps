import { Show, lazy, onMount } from "solid-js";
import { HashRouter, useLocation } from "@solidjs/router";
import Sidebar from "./shell/Sidebar";
import About from "./routes/about/index.jsx";
import { AiPanel } from "@/components/ai-panel";
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
  const isAiSelected = () => location.pathname.startsWith("/ai");

  return (
    <div class="shell">
      <div class="col-sidebar">
        <Sidebar />
      </div>
      <div class="col-center">
        <div style={{ display: isAiSelected() ? "none" : undefined }}>
          {props.children}
        </div>
        <Show when={aichat.available}>
          <div style={{ display: isAiSelected() ? undefined : "none" }}>
            <AiPanel />
          </div>
        </Show>
      </div>
      <About />
    </div>
  );
}

/** @type {import("@solidjs/router").RouteDefinition[]} */
const routes = [
  { path: "/", component: lazy(() => import("./routes/home/index.jsx")) },
  { path: "/ai", component: lazy(() => import("./routes/ai/index.jsx")) },
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
