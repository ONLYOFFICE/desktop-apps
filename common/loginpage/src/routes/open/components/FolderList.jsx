import { For, Show } from "solid-js";
import { openFolder } from "@/sdk";
import { t } from "@/services/l10n";
import "./FolderList.styles.css";
import { usePngIcons } from "@/services/display";

function FolderRow(props) {
  return (
    <div class="row text-normal" classList={{ pinned: props.folder.pinned }} onClick={() => openFolder(props.folder.full)}>
      <div class="col-name" title={props.folder.name}>
        <div class="icon">
          <svg class="icon">
            <use href="#foldergray" />
          </svg>
        </div>
        <p class="name">{props.folder.name}</p>
      </div>
      <div class="col-location" title={props.folder.descr}>
        {props.folder.descr}
      </div>
      <div class="col-actions">
        <button
          class="pin"
          onClick={(e) => {
            e.stopPropagation();
            props.onTogglePin(props.folder.full);
          }}
        >
          {/* <svg class="icon">
            <use href="#pin" />
          </svg> */}
          <Show when={usePngIcons()} fallback={<svg class="icon"><use href="#pin" /></svg>}>
            <i class="icon tool-icon pin" />
          </Show>
        </button>
      </div>
    </div>
  );
}

export function FolderList(props) {
  const folders = () => props.folders || [];
  const hasFolders = () => folders().length > 0;

  return (
    <Show when={hasFolders()}>
      <div id="box-recent-folders">
        <div class="file-list-head text-normal">
          <div class="col-name">{t("colFileName")}</div>
          <div class="col-location">{t("colLocation")}</div>
        </div>
        <div class="file-list-body scrollable">
          <For each={folders()}>{(folder) => <FolderRow folder={folder} onTogglePin={props.onTogglePin} />}</For>
        </div>
      </div>
    </Show>
  );
}
