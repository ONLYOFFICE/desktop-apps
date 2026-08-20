import { For, Show, createSignal, createEffect } from "solid-js";
import { providers, portals, openConnectedPortal, logoutConnectedPortal, forgetConnectedPortal, connectToProvider } from "@/services/portals.js";
import { relpath } from "@/sdk/portals.js";
import { command } from "@/sdk/bridge.js";
import { t } from "@/services/l10n.js";
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
  const [error, setError] = createSignal("");
  const [checking, setChecking] = createSignal(false);
  const hasEntryPage = () => !!props.provider?.entryPage;
  const isOnlyoffice = () => props.provider?.provider === "onlyoffice";
  let inputRef;

  createEffect(() => {
    setUrl(props.provider?.entryPage ?? "");
    setError("");
    if (props.provider && !hasEntryPage()) {
      setTimeout(() => inputRef?.focus(), 150);
    }
  });

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
    setError("");
    props.onClose();
  }

  function handleCreatePortal() {
    command("portal:create", "");
    handleClose();
  }

  function isValidUrl(str) {
    try { new URL(str); return true; } catch { return false; }
  }

  function getCandidates(raw) {
    const s = raw.trim();
    if (!s) return [];
    if (/^https?:\/\//i.test(s)) return [s];
    return [`https://${s}`, `http://${s}`];
  }

  async function handleConnect() {
    if (checking()) return;
    if (hasEntryPage()) {
      connectToProvider(props.provider, props.provider.entryPage);
      handleClose();
    }

    const candidates = getCandidates(url()).filter(isValidUrl);
    if (!candidates.length) {
      setError(t("errInvalidUrl", "Invalid URL"));
      return;
    }
    const checkPath = props.provider?.check?.url ?? "";
    setChecking(true);
    try {
      for (const candidate of candidates) {
        try {
          await fetch('onlyoffice-proxy://' + candidate + checkPath, { method: "HEAD", signal: AbortSignal.timeout(5000) });
          connectToProvider(props.provider, candidate);
          handleClose();
          return;
        } catch {}
      }
      setError(t("errServerNotAccessible", "Server is not accessible"));
    } finally {
      setChecking(false);
    }
  }

  return (
    <Dialog open={!!props.provider} onOpenChange={(open) => !open && handleClose()}>
      <Dialog.Content class="connect-dialog" onKeyDown={(e) => e.key === "Enter" && e.target.tagName !== "BUTTON" && handleConnect()}>
        <div class="dlg-header">
          <Dialog.Title class="dlg-title">
            {t("dlgConnectToTitle", "Connect to $1").replace("$1", props.provider?.name ?? "")}
          </Dialog.Title>
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
            <label class="connect-label">{t("pshPortal")}</label>
            <div class="url-field-wrap" classList={{ "url-field-wrap--error": !!error() }}>
              <TextField
                value={url()}
                onChange={(val) => { setUrl(val); setError(""); }}
                disabled={hasEntryPage()}
                class="connect-field"
              >
                <TextField.Input
                  ref={inputRef}
                  placeholder="https://"
                />
              </TextField>
              <Show when={error()}>
                <span class="url-error-icon" title={error()}>
                  <svg class="icon"><use href="#warning" /></svg>
                </span>
              </Show>
            </div>
            <Show when={isOnlyoffice()}>
              <button type="button" class="connect-link" onClick={handleCreatePortal}>
                {t("linkNoPortal", "Don't have an ONLYOFFICE cloud? Create one")}
              </button>
            </Show>
          </div>
        </div>

        <div class="connect-footer">
          <Button onClick={handleClose}>{t("btnCancel", "Cancel")}</Button>
          <Button tabIndex="1" variant="accent" onClick={handleConnect} disabled={checking()}>{t("btnConnectAction", "Connect")}</Button>
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
          <DropdownMenu.Content class="iconless">
            <DropdownMenu.Item onSelect={() => openConnectedPortal(props.portal)}>{t("menuFileOpen")}</DropdownMenu.Item>
            <DropdownMenu.Item aria-disabled={props.portal.logged === false} onSelect={() => props.portal.logged !== false && logoutConnectedPortal(props.portal)}>{t("menuLogout")}</DropdownMenu.Item>
            <DropdownMenu.Item onSelect={() => props.onRemove(props.portal)}>{t("menuRemoveModel")}</DropdownMenu.Item>
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
      <Button variant="default" class="provider-connect-btn" onClick={() => props.onConnect(props.provider)}>{t("btnConnectAction", "Connect")}</Button>
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
          <h2 class="clouds-section-title">{t("portalListTitle")}</h2>
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
        <h2 class="clouds-section-title">{t("portalAvailableTitle", "Available clouds")}</h2>
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
