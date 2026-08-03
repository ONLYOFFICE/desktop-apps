import { For, Show } from "solid-js";
import { t } from "@/services/l10n";
import { ContextMenu, DropdownMenu } from "@/components";
import { usePngIcons } from "@/services/display";
import "./RecentList.styles.css";

function RecoverMenuItems(props) {
  const Menu = props.menu;
  return (
    <>
      <Menu.Item icon="#open" onSelect={() => props.onOpen(props.model)}>
        {t("menuOpen", "Open")}
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

function RecoverRow(props) {
  return (
    <ContextMenu>
      <ContextMenu.Trigger
        as="div"
        class="row text-normal"
        onClick={() => props.onOpen(props.model)}
      >
        <div class="col-name" title={props.model.fullName}>
          <div class="icon">
            <Show
              when={usePngIcons()}
              fallback={<svg class="icon"><use href={`#${props.model.format}`} /></svg>}
            >
              <i class={`doc-format ${props.model.format}`} />
            </Show>
          </div>
          <p class="name">
            {props.model.name}
            <span class="ext">{props.model.ext}</span>
          </p>
        </div>
        <div class="col-actions">
          <DropdownMenu>
            <DropdownMenu.Trigger class="btn-recents-tool more" onClick={(e) => e.stopPropagation()}>
              <Show when={usePngIcons()} fallback={<svg class="icon"><use href="#more" /></svg>}>
                <i class="icon tool-icon more" />
              </Show>
            </DropdownMenu.Trigger>
            <DropdownMenu.Content onClick={(e) => e.stopPropagation()} onContextMenu={(e) => e.stopPropagation()}>
              <RecoverMenuItems
                menu={DropdownMenu}
                model={props.model}
                onOpen={props.onOpen}
                onRemove={props.onRemove}
                onClear={props.onClear}
              />
            </DropdownMenu.Content>
          </DropdownMenu>
        </div>
      </ContextMenu.Trigger>
      <ContextMenu.Content>
        <RecoverMenuItems
          menu={ContextMenu}
          model={props.model}
          onOpen={props.onOpen}
          onRemove={props.onRemove}
          onClear={props.onClear}
        />
      </ContextMenu.Content>
    </ContextMenu>
  );
}

export function RecoverList(props) {
  const recovers = () => props.recovers || [];

  return (
    <Show when={recovers().length > 0}>
      <div id="box-recovery">
        <div class="list-section-title">{t("recovers", "Recovers")}</div>
        <div class="file-list-head text-normal">
          <div class="col-name">{t("colFileName")}</div>
        </div>
        <div class="file-list-body">
          <For each={recovers()}>
            {(model) => (
              <RecoverRow
                model={model}
                onOpen={props.onOpen}
                onRemove={props.onRemove}
                onClear={props.onClear}
              />
            )}
          </For>
        </div>
      </div>
    </Show>
  );
}
