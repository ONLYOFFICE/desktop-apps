import { Show, createSignal, onCleanup, onMount } from "solid-js";
import { createStore, reconcile } from "solid-js/store";
import { t } from "@/services/l10n.js";
import { listRecents,
        onRecentsChanged,
        onRecoversChanged,
        onNativeMessage,
        parseRecent,
        pinRecent,
        removeRecent,
        clearRecents,
        checkFiles,
        listRecovers,
        removeRecover,
        clearRecovers,
        openRecoveryFile,
        appReady } from "@/sdk";
import { DocTypeGrid } from "./components/DocTypeGrid.jsx";
import { RecentList } from "./components/RecentList.jsx";
import { RecoverList } from "./components/RecoverList.jsx";
import { FileDropZone } from "./components/FileDropZone.jsx";
import { aichat, openFullAIPanel, openSidePanel } from "@/stores/aichat.js";
import { command } from "@/sdk/bridge.js";
import "@/styles/components/home.css";
import { sendChatRecentsChanged, sendFileStatusToChat, sendFileToChat, sendPluginVisibility } from "@/services/ai-panel.js";

const toArray = (v) => (Array.isArray(v) ? v : []);

function sortByPin(arr) {
  arr.sort((a, b) => a.pinid - b.pinid);
  return arr;
}

const BATCH = 40;

export default function Index() {
  const [recents, setRecents] = createStore([]);
  const [recovers, setRecovers] = createSignal([]);
  const [loaded, setLoaded] = createSignal(false);
  const [visibleCount, setVisibleCount] = createSignal(BATCH);
  let revealId = 0;
  let recentsCountChanged = true;

  const hasFiles = () => recents.length > 0;
  const hasRecovers = () => recovers().length > 0;
  const visibleRecents = () => recents.slice(0, visibleCount());

  function loadRecents(raw) {
    const sorted = sortByPin(parseRecent(toArray(raw)));

    if (!recentsCountChanged)
      recentsCountChanged = sorted.length === recents.length;

    const id = ++revealId;
    setVisibleCount(Math.min(BATCH, sorted.length));
    setRecents(reconcile(sorted, { key: "uid" }));

    if (sorted.length > BATCH) {
      let shown = BATCH;
      function reveal() {
        if (id !== revealId)
          return;

        shown = Math.min(shown + BATCH, sorted.length);
        setVisibleCount(shown);
        if (shown < sorted.length) {
          requestAnimationFrame(reveal);
        } else if (recentsCountChanged) {
          sendCheckFiles(sorted, id);
        }
      }
      requestAnimationFrame(reveal);
    } else if (recentsCountChanged) {
      sendCheckFiles(sorted, id);
    }
  }

  function sendCheckFiles(files, id, start = 0) {
    if (id !== revealId || start >= files.length) return;

    const end = Math.min(start + BATCH, files.length);
    const pathByHash = {};
    for (let i = start; i < end; i++) {
      pathByHash[files[i].hash] = files[i].path;
    }
    checkFiles(pathByHash);

    if (end < files.length) {
      requestAnimationFrame(() => sendCheckFiles(files, id, end));
    } else {
      recentsCountChanged = false;
    }
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
    const sorted = sortByPin(arr);
    setRecents(reconcile(sorted, { key: "uid" }));
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

  function loadRecovers(raw) {
    setRecovers(parseRecent(toArray(raw)));
  }

  function openRecover(model) {
    openRecoveryFile(model);
  }

  function removeRecoverItem(fileid) {
    removeRecover(fileid);
    setRecovers(recovers().filter((r) => r.fileid !== fileid));
  }

  function clearAllRecovers() {
    clearRecovers();
    setRecovers([]);
  }

  function addToChat(model) {
    if (aichat.available) {
      // navigate("/ai");
      sendFileToChat(model.path, model.type, model.fullName);
      if (!aichat.expanded) {
        sendPluginVisibility("minimized");
        openSidePanel();
      } else {
        sendPluginVisibility("expanded");
        openFullAIPanel();
      }
    } else {
      command('addon:action', 'intro');
    }
  }

  onMount(() => {
    listRecents()
      .then(() => {
        setLoaded(true);
      })
      .catch(() => setLoaded(true));

    listRecovers().then(loadRecovers).catch(() => {});

    const unsubRecents = onRecentsChanged((p) => {
      loadRecents(p);
      sendChatRecentsChanged();
      if (!loaded()) setLoaded(true);
    });

    const unsubRecovers = onRecoversChanged(loadRecovers);

    const unsubCommonMsgs = onNativeMessage((msg, param) => {
      if (msg === "files:checked") {
        let checked;
        try {
          checked = JSON.parse(param);
        } catch {
          return;
        }

        if (!checked || typeof checked !== "object")
          return;

        for (const hash in checked) {
          const hash_number = parseInt(hash);
          const idx = recents.findIndex(i => i.hash == hash_number);

          if (!(idx < 0)) {
            setRecents(idx, "exist", checked[hash]);
            sendFileStatusToChat(recents[idx]);
          }
        }
      }
    });

    onCleanup(() => {
      ++revealId;
      unsubRecents();
      unsubRecovers();
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
        <RecoverList
          recovers={recovers()}
          onOpen={openRecover}
          onRemove={removeRecoverItem}
          onClear={clearAllRecovers}
        />
        <RecentList
          recents={visibleRecents()}
          heading={hasRecovers() ? t("listRecentFileTitle", "Recents") : undefined}
          onTogglePin={togglePin}
          onRemove={markFileRemoved}
          onClear={clearAll}
          onAddToChat={addToChat}
        />
      </section>
    </div>
  );
}
