import { Show, For, lazy, createEffect, createSignal, onMount } from "solid-js";
import { HashRouter, useLocation } from "@solidjs/router";
import Sidebar from "./shell/Sidebar";
import About from "./routes/about/index.jsx";
import { PluginModal } from "@/components/plugin-modal/PluginModal";
import HomePanel from "./routes/home/index.jsx";
import { AiPanel } from "@/components/ai-panel";
import { ResizeHandle } from "@/components/ai-panel/ResizeHandle.jsx";
import { signalAppReady } from "@/sdk";
import { initSettingsSync } from "@/services/settings-sync";
import { initAbout } from "@/services/about";
import { initPortals } from "@/services/portals";
import { initAiPanel } from "@/services/ai-panel";
import { aichat } from "@/stores/aichat";
import { extPlugins, pluginRouteId } from "@/stores/ext-plugins";

const TemplatesPanel = lazy(() => import("./routes/templates/index.jsx"));
import "./styles/main.css";
import "./styles/layout.css";

function Shell(props) {
  const location = useLocation();
  const isHomeSelected = () => location.pathname === "/";
  const isTemplatesSelected = () => location.pathname.startsWith("/templates");
  const [templatesMounted, setTemplatesMounted] = createSignal(false);
  createEffect(() => { if (isTemplatesSelected()) setTemplatesMounted(true); });
  const isChatSelected = () => location.pathname === "/aichat";
  const isAiToolsSelected = () => location.pathname.startsWith("/aitools");
  const isSettingsSelected = () => location.pathname.startsWith("/settings");
  const showSidePanel = () => aichat.available && aichat.open && !isChatSelected() && /*!isAiToolsSelected() &&*/ !isSettingsSelected();
  const isChatExpanded = () => isChatSelected() && aichat.available;
  const showAiCol = () => showSidePanel() || isChatExpanded();

  createEffect(() => {
    document.body.style.setProperty("--chat-w", `${aichat.width}px`);
  });

  return (
    <div
      class="shell"
      classList={{
        "shell--chat-panel-side": showSidePanel(),
        "shell--chat-panel-full": isChatExpanded(),
      }}
    >
      <div class="col-sidebar">
        <Sidebar />
      </div>
      <div class="col-center">
        <div class="home-panel" classList={{ "panel-active": isHomeSelected() }}>
          <HomePanel />
        </div>
        {props.children}
        <Show when={templatesMounted()}>
          <div class="templates-panel" classList={{ "panel-active": isTemplatesSelected() }}>
            <TemplatesPanel />
          </div>
        </Show>
        <div class="ext-plugin-frame" classList={{ "ext-plugin-frame--active": isAiToolsSelected() }}>
          <iframe src="https://maxkadushkin.github.io/underconstruction/" title="ONLYOFFICE" />
        </div>
        <For each={extPlugins()}>
          {(plugin) => {
            const isActive = () =>
              location.pathname === `/plugin/${pluginRouteId(plugin.id)}`;
            return (
              <div
                class="ext-plugin-frame"
                classList={{ "ext-plugin-frame--active": isActive() }}
              >
                <iframe src={plugin.url} title={plugin.name} />
              </div>
            );
          }}
        </For>
      </div>
      <Show when={aichat.available}>
        <div class="col-ai" style={{ display: showAiCol() ? undefined : "none" }}>
          <Show when={showSidePanel()}>
            <ResizeHandle />
          </Show>
          <AiPanel />
        </div>
      </Show>
      <About />
      <PluginModal />
    </div>
  );
}

/** @type {import("@solidjs/router").RouteDefinition[]} */
const routes = [
  { path: "/", component: () => null },
  { path: "/aichat", component: lazy(() => import("./routes/aichat/index.jsx")) },
  { path: "/aitools", component: lazy(() => import("./routes/aitools/index.jsx")) },
  { path: "/plugin/:pluginId", component: lazy(() => import("./routes/plugin/index.jsx")) },
  { path: "/open", component: lazy(() => import("./routes/open/index.jsx")) },
  { path: "/templates", component: () => null },
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
