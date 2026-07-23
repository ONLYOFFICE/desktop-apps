import { Show, createSignal, onCleanup, onMount } from "solid-js";
import { createStore, reconcile } from "solid-js/store";
import { t } from "@/services/l10n.js";
import { listRecents, onRecentsChanged, onNativeMessage, parseRecent, pinRecent, removeRecent, clearRecents, checkFiles, appReady } from "@/sdk";
import { DocTypeGrid } from "./components/DocTypeGrid.jsx";
import { RecentList } from "./components/RecentList.jsx";
import { FileDropZone } from "./components/FileDropZone.jsx";
import { aichat } from "@/stores/aichat.js";
import { command } from "@/sdk/bridge.js";
import "@/styles/components/home.css";
import { sendPluginVisibility } from "@/services/ai-panel.js";

const toArray = (v) => (Array.isArray(v) ? v : []);

function sortByPin(arr) {
  arr.sort((a, b) => a.pinid - b.pinid);
  return arr;
}

const BATCH = 40;

export default function Index() {
  const [recents, setRecents] = createStore([]);
  const [loaded, setLoaded] = createSignal(false);
  const [visibleCount, setVisibleCount] = createSignal(BATCH);
  let revealId = 0;

  const hasFiles = () => recents.length > 0;
  const visibleRecents = () => recents.slice(0, visibleCount());

  function loadRecents(raw) {
    const id = ++revealId;
    const sorted = sortByPin(parseRecent(toArray(raw)));
    setVisibleCount(Math.min(BATCH, sorted.length));
    setRecents(reconcile(sorted, { key: "uid" }));

    if (sorted.length > BATCH) {
      let shown = BATCH;
      function reveal() {
        if (id !== revealId) return;
        shown = Math.min(shown + BATCH, sorted.length);
        setVisibleCount(shown);
        if (shown < sorted.length) requestAnimationFrame(reveal);
      }
      requestAnimationFrame(reveal);
    }
  }

  function sendCheckFiles() {
    const pathByHash = {};
    recents.forEach((f) => {
      pathByHash[f.hash] = f.path;
    });
    Object.keys(pathByHash).length && checkFiles(pathByHash);
  }

  function togglePin(fileid) {
    const arr = recents.map((r) => {
      if (r.fileid === fileid) {
        const newPinned = !r.pinned;
        pinRecent(fileid, newPinned);
        return { ...r, pinned: newPinned, pinid: newPinned ? -r.fileid : r.fileid };
      }
      return r;
    });
    setRecents(reconcile(sortByPin(arr), { key: "uid" }));
  }

  function markFileRemoved(fileid) {
    removeRecent(fileid);
    setRecents(
      reconcile(
        recents.map((r) => (r.fileid === fileid ? { ...r, exist: false } : r)),
        { key: "uid" },
      ),
    );
  }

  function clearAll() {
    clearRecents();
    setRecents(reconcile([], { key: "uid" }));
  }

  function addToChat() {
    if (aichat.available) {
      // navigate("/ai");
      sendPluginVisibility("minimized");
    } else {
      command('addon:action', 'intro');
    }
  }

  onMount(() => {
    listRecents()
      .then((raw) => {
        loadRecents(raw);
        setLoaded(true);
        if (appReady()) sendCheckFiles();
      })
      .catch(() => setLoaded(true));

    const unsubRecents = onRecentsChanged((p) => {
      loadRecents(p);
      if (!loaded()) setLoaded(true);
    });

    const unsubCommonMsgs = onNativeMessage((msg, param) => {
      if (msg === "app:ready") {
        sendCheckFiles();
      } else if (msg === "files:checked") {
        let checked;
        try {
          checked = JSON.parse(param);
        } catch {
          return;
        }
        if (!checked || typeof checked !== "object") return;
        setRecents(
          reconcile(
            recents.map((r) => (r.hash in checked ? { ...r, exist: checked[r.hash] } : r)),
            { key: "uid" },
          ),
        );
      }
    });

    onCleanup(() => {
      ++revealId;
      unsubRecents();
      unsubCommonMsgs();
    });
  });

  return (
    <div class="home-page">
      <div class="cnt-title">
        <h1>{t("actCreateNew")}</h1>
      </div>

      <section class="new-doc-section">
        <DocTypeGrid />
      </section>

      <Show when={loaded() && !hasFiles()}>
        <section class="file-drop-section">
          <FileDropZone />
        </section>
      </Show>

      <section class="file-list-section">
        <RecentList recents={visibleRecents()}
                    onTogglePin={togglePin}
                    onRemove={markFileRemoved}
                    onClear={clearAll}
                    onAddToChat={addToChat} />
      </section>
    </div>
  );
}
