import { createEffect, createSignal, For, onCleanup, Show } from "solid-js";
import { exploreFile, openRecentFile } from "@/sdk";
import { t } from "@/services/l10n";
import { ContextMenu, DropdownMenu } from "@/components";
import { usePngIcons } from "@/services/display";
import "./RecentList.styles.css";
import { appliedUseAi } from "@/services/settings-sync";

function FileMenuItems(props) {
  const Menu = props.menu;
  return (
    <>
      <Show when={props.model.islocal && appliedUseAi()}>
        <Menu.Item icon="#aichat20" onSelect={() => props.onAddToChat(props.model)}>
          {t("actAiChat")}
        </Menu.Item>
      </Show>
      <Menu.Item icon="#gofolder" onSelect={() => exploreFile(props.model)}>
        {t("menuFileExplore")}
      </Menu.Item>
      <Menu.Item icon={props.model.pinned ? "#unpin20" : "#pin20"} onSelect={() => props.onTogglePin(props.model.fileid)}>
        {t(props.model.pinned ? "menuFileUnpin" : "menuFilePin")}
      </Menu.Item>
      <Menu.Item icon="#remove" onSelect={() => props.onRemove(props.model.fileid)}>
        {t("menuRemoveModel")}
      </Menu.Item>
      <Menu.Separator />
      <Menu.Item class="negative" onSelect={props.onClear}>
        {t("menuClear")}
      </Menu.Item>
    </>
  );
}

function RecentRow(props) {
  const [menuOpen, setMenuOpen] = createSignal(false);

  let rowEl;
  let moreEl;

  const isMoreHidden = () =>
    moreEl.offsetParent === null || getComputedStyle(moreEl).visibility === "hidden";

  createEffect(() => {
    if (!menuOpen() || !rowEl || !moreEl) return;

    const observer = new ResizeObserver(() => {
      if (isMoreHidden()) setMenuOpen(false);
    });

    observer.observe(rowEl);
    onCleanup(() => observer.disconnect());
  });

  return (
    <ContextMenu>
      <ContextMenu.Trigger
        as="div"
        ref={(el) => (rowEl = el)}
        class="row text-normal"
        classList={{
          pinned: props.model.pinned,
          lost: props.model.exist === false,
        }}
        onClick={() => {
          openRecentFile(props.model);
        }}
      >
        <div class="col-name" title={props.model.fullName}>
          <div class="icon">
            <Show
              when={usePngIcons() && props.model.type !== "folder"}
              fallback={<svg class="icon"><use href={`#${props.model.type === "folder" ? "folder-small" : props.model.format}`} /></svg>}
            >
              <i class={`doc-format ${props.model.format}`} />
            </Show>
            <Show when={props.model.crypted}>
              <svg class="icon shield">
                <use href="#shield" />
              </svg>
            </Show>
          </div>
          <p class="name">
            {props.model.name}
            <span class="ext">{props.model.ext}</span>
          </p>
        </div>
        <div class="col-location" title={props.model.descr}>
          {props.model.descr}
        </div>
        <Show when={props.model.type !== "folder"}>
          <div class="col-date" title={props.model.date}>
            <p>{props.model.dateLabel}</p>
          </div>
        </Show>
        <div class="col-actions">
          <Show when={props.model.islocal && appliedUseAi()}>
            <button
              class="btn-recents-tool aichat"
              onClick={(e) => {
                e.stopPropagation();
                props.onAddToChat(props.model);
              }}
            >
              <Show when={usePngIcons()} fallback={<svg class="icon"><use href="#aichat_rcl" /></svg>}>
                <i class="icon tool-icon aichat" />
              </Show>
            </button>
          </Show>
          <button
            class="btn-recents-tool pin"
            classList={{ pinned: props.model.pinned }}
            onClick={(e) => {
              e.stopPropagation();
              props.onTogglePin(props.model.fileid);
            }}
          >
            <Show when={usePngIcons()}
              fallback={
                <>
                  <svg class="icon default"><use href={props.model.pinned ? "#pinned" : "#pin"} /></svg>
                  <svg class="icon hover"><use href="#unpin" /></svg>
                </>
              }
            >
              <i class={`icon tool-icon default ${props.model.pinned ? "pinned" : "pin"}`} />
              <i class="icon tool-icon hover unpin" />
            </Show>
          </button>
          <Show when={props.model.type !== "folder"}>
            <DropdownMenu open={menuOpen()} onOpenChange={setMenuOpen}>
              <DropdownMenu.Trigger ref={(el) => (moreEl = el)} class="btn-recents-tool more" onClick={(e) => e.stopPropagation()}>
                <Show when={usePngIcons()} fallback={<svg class="icon"><use href="#more" /></svg>}>
                  <i class="icon tool-icon more" />
                </Show>
              </DropdownMenu.Trigger>
              <DropdownMenu.Content onClick={(e) => e.stopPropagation()} onContextMenu={(e) => e.stopPropagation()}>
                <FileMenuItems
                  menu={DropdownMenu}
                  model={props.model}
                  onAddToChat={props.onAddToChat}
                  onTogglePin={props.onTogglePin}
                  onRemove={props.onRemove}
                  onClear={props.onClear}
                />
              </DropdownMenu.Content>
            </DropdownMenu>
          </Show>
        </div>
      </ContextMenu.Trigger>
      <ContextMenu.Content>
        <FileMenuItems
          menu={ContextMenu}
          model={props.model}
          onAddToChat={props.onAddToChat}
          onTogglePin={props.onTogglePin}
          onRemove={props.onRemove}
          onClear={props.onClear}
        />
      </ContextMenu.Content>
    </ContextMenu>
  );
}

export function RecentList(props) {
  const recents = () => props.recents || [];
  const hasRecents = () => recents().length > 0;

  return (
    <Show when={hasRecents()}>
      <div id="box-recent">
        <Show when={props.heading}>
          <div class="list-section-title">{props.heading}</div>
        </Show>
        <div class="file-list-head text-normal">
          <div class="col-name">{t("colFileName")}</div>
          <div class="col-location">{t("colLocation")}</div>
          <div class="col-date">{t("colLastOpened")}</div>
        </div>
        <div class="file-list-body scrollable">
          <For each={recents()}>
            {(model) => <RecentRow model={model}
                                onTogglePin={props.onTogglePin}
                                onRemove={props.onRemove}
                                onClear={props.onClear}
                                onAddToChat={props.onAddToChat} />}
          </For>
        </div>
      </div>
    </Show>
  );
}
