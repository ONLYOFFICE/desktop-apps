import { Show, createSignal, onCleanup, onMount } from "solid-js";
import { createStore, reconcile } from "solid-js/store";
import { t } from "@/services/l10n.js";
import { listRecents, onRecentsChanged, parseRecentDirs, togglePinnedFolder, openLocal } from "@/sdk";
import { Button } from "@/components";
import { FileDropZone } from "../home/components/FileDropZone.jsx";
import { FolderList } from "./components/FolderList.jsx";
import "./open.css";

function sortByPin(arr) {
  return [...arr].sort((a, b) => a.pinid - b.pinid);
}

export default function Open() {
  const [folders, setFolders] = createStore([]);
  const [loaded, setLoaded] = createSignal(false);

  const hasFolders = () => folders.length > 0;

  function loadFolders(raw) {
    setFolders(reconcile(sortByPin(parseRecentDirs(raw, t)), { key: "uid" }));
  }

  function togglePin(path) {
    togglePinnedFolder(path);
    const updated = folders.map((f) => {
      if (f.full === path) return { ...f, pinned: !f.pinned, pinid: -f.pinid };
      return f;
    });
    setFolders(reconcile(sortByPin(updated), { key: "uid" }));
  }

  onMount(() => {
    listRecents()
      .then((raw) => {
        loadFolders(raw);
        setLoaded(true);
      })
      .catch(() => setLoaded(true));

    const unsub = onRecentsChanged((p) => {
      loadFolders(p);
      if (!loaded()) setLoaded(true);
    });

    onCleanup(() => unsub());
  });

  return (
    <div class="open-page">
      <div class="cnt-title">
        <h1>{t("listRecentDirTitle")}</h1>
      </div>

      <Show when={loaded() && !hasFolders()}>
        <section class="file-drop-section">
          <FileDropZone />
        </section>
      </Show>

      <Show when={hasFolders()}>
        <>
          <section class="file-list-section">
            <FolderList folders={folders} onTogglePin={togglePin} />
          </section>
          <div class="open-actions">
            <Button variant="accent" onClick={openLocal}>
              {t("btnBrowse")}
            </Button>
          </div>
        </>
      </Show>
    </div>
  );
}
