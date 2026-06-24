import { For, Show, createSignal } from "solid-js";
import { providers, portals, openConnectedPortal, logoutConnectedPortal, forgetConnectedPortal, connectToProvider } from "@/services/portals.js";
import { relpath } from "@/sdk/portals.js";
import { Button, Dialog, DropdownMenu, TextField } from "@/components";
import "./clouds.css";

function ProviderIcon(props) {
  const paths = () => {
    const p = props.provider;
    if (!p?.icons?.themeLight) return null;
    const base = `${relpath}/providers/${p.provider}`;
    const lightFile = p.icons.themeLight[props.type];
    if (!lightFile) return null;
    return {
      light: `${base}/${lightFile}`,
      dark: p.icons.themeDark?.[props.type] ? `${base}/${p.icons.themeDark[props.type]}` : null,
    };
  };

  return (
    <div class="provider-icon">
      <Show
        when={paths()}
        fallback={
          <svg class="icon">
            <use href="#cloud" />
          </svg>
        }
      >
        <img class="icon icon__light" src={paths().light} />
        <Show when={paths().dark}>
          <img class="icon icon__dark" src={paths().dark} />
        </Show>
      </Show>
    </div>
  );
}

function ConnectDialog(props) {
  const [url, setUrl] = createSignal("");

  const logoPaths = () => {
    const p = props.provider;
    if (!p?.icons?.themeLight) return null;
    const base = `${relpath}/providers/${p.provider}`;
    const lightFile = p.icons.themeLight.buttonLogo;
    if (!lightFile) return null;
    return {
      light: `${base}/${lightFile}`,
      dark: p.icons.themeDark?.buttonLogo ? `${base}/${p.icons.themeDark.buttonLogo}` : null,
    };
  };

  function handleClose() {
    setUrl("");
    props.onClose();
  }

  function handleConnect() {
    const val = url().trim();
    if (!val) return;
    connectToProvider(props.provider, val);
    handleClose();
  }

  return (
    <Dialog open={!!props.provider} onOpenChange={(open) => !open && handleClose()}>
      <Dialog.Content class="connect-dialog">
        <div class="dlg-header">
          <Dialog.Title class="dlg-title">Connect to {props.provider?.name}</Dialog.Title>
          <Dialog.CloseButton class="dlg-close" />
        </div>

        <div class="connect-inner">
          <Show when={logoPaths()}>
            <div class="connect-logo">
              <img class="icon__light" src={logoPaths().light} />
              <Show when={logoPaths().dark}>
                <img class="icon__dark" src={logoPaths().dark} />
              </Show>
            </div>
          </Show>

          <div class="connect-body">
            <label class="connect-label">Cloud office URL</label>
            <TextField value={url()} onChange={setUrl} class="connect-field">
              <TextField.Input placeholder="https://" />
            </TextField>
          </div>
        </div>

        <div class="connect-footer">
          <Button onClick={handleClose}>Cancel</Button>
          <Button variant="accent" onClick={handleConnect}>Connect</Button>
        </div>
      </Dialog.Content>
    </Dialog>
  );
}

function PortalRow(props) {
  const [menuOpen, setMenuOpen] = createSignal(false);
  const providerData = () => providers().find((p) => p.provider === props.portal.provider);
  const displayName = () => props.portal.portal.replace(/^https?:\/\//, "");
  const userInfo = () => (props.portal.email ? `${props.portal.user} (${props.portal.email})` : props.portal.user);

  return (
    <div class="portal-row" onClick={() => !menuOpen() && openConnectedPortal(props.portal)}>
      <ProviderIcon provider={providerData()} type="connectionsList" />
      <div class="portal-info">
        <p class="portal-name" title={props.portal.portal}>
          {displayName()}
        </p>
        <p class="portal-user">{userInfo()}</p>
      </div>
      <div class="portal-actions">
        <DropdownMenu placement="bottom-end" onOpenChange={setMenuOpen}>
          <DropdownMenu.Trigger class="btn-portal-action" onClick={(e) => e.stopPropagation()}>
            <svg class="icon">
              <use href="#more" />
            </svg>
          </DropdownMenu.Trigger>
          <DropdownMenu.Content>
            <DropdownMenu.Item onSelect={() => openConnectedPortal(props.portal)}>Open</DropdownMenu.Item>
            <DropdownMenu.Item aria-disabled={props.portal.logged === false} onSelect={() => props.portal.logged !== false && logoutConnectedPortal(props.portal)}>Logout</DropdownMenu.Item>
            <DropdownMenu.Item onSelect={() => props.onRemove(props.portal)}>Remove</DropdownMenu.Item>
          </DropdownMenu.Content>
        </DropdownMenu>
      </div>
    </div>
  );
}

function ProviderItem(props) {
  return (
    <div class="provider-item">
      <ProviderIcon provider={props.provider} type="connectionsList" />
      <span class="provider-name">{props.provider.name}</span>
      <Button variant="default" class="provider-connect-btn" onClick={() => props.onConnect(props.provider)}>Connect</Button>
    </div>
  );
}

const REMOVE_DELAY = 300;

export default function Clouds() {
  const [connectingProvider, setConnectingProvider] = createSignal(null);
  const [removingIds, setRemovingIds] = createSignal(new Set());

  function handleRemove(portal) {
    if (removingIds().has(portal.portal)) return;
    setRemovingIds((prev) => new Set([...prev, portal.portal]));
    setTimeout(() => {
      forgetConnectedPortal(portal);
      setRemovingIds((prev) => {
        const next = new Set(prev);
        next.delete(portal.portal);
        return next;
      });
    }, REMOVE_DELAY);
  }

  return (
    <div class="clouds-page">
      <Show when={portals().length > 0}>
        <section>
          <h2 class="clouds-section-title">Connected clouds</h2>
          <div class="portal-list">
            <For each={portals()}>
              {(portal) => (
                <div class="portal-item-wrap" classList={{ removing: removingIds().has(portal.portal) }}>
                  <PortalRow portal={portal} onRemove={handleRemove} />
                </div>
              )}
            </For>
          </div>
        </section>
      </Show>

      <section>
        <h2 class="clouds-section-title">Available clouds</h2>
        <div class="provider-list">
          <For each={providers()}>
            {(provider) => <ProviderItem provider={provider} onConnect={setConnectingProvider} />}
          </For>
        </div>
      </section>

      <ConnectDialog provider={connectingProvider()} onClose={() => setConnectingProvider(null)} />
    </div>
  );
}
