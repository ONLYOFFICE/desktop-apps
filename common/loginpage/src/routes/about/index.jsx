import { Show } from "solid-js";
import { t } from "@/services/l10n.js";
import { aboutOpen, closeAbout, versionInfo, updatesOn, updateStatus, execUpdateAction, addonInfo } from "@/services/about.js";
import { Dialog, Button } from "@/components";
import "./about.css";

export default function About() {
  return (
    <Dialog open={aboutOpen()} onOpenChange={(isOpen) => !isOpen && closeAbout()}>
      <Dialog.Content class="about-dialog">
        <div class="dlg-header">
          <Dialog.Title class="dlg-title">{t("actAbout")}</Dialog.Title>
          <Dialog.CloseButton class="dlg-close" />
        </div>
        <div class="dlg-body">
          <Show when={versionInfo()}>
            <div class="about-content">
              <section class="about-approw">
                <div class={`about-logo ${versionInfo().logocls ?? ""}`}>
                  <svg class="ver-logo">
                    <use href="#idx-logo-light" />
                    <use href="#idx-logo-dark" />
                  </svg>
                </div>
                <div class="about-appinfo">
                  <p class="about-appname">{versionInfo().appname}</p>
                  <p class="about-version">
                    <span class="about-edition-label">
                      {versionInfo().commercial ? t("strVersionCommercial") : t("strVersionCommunity")}
                    </span>{" "}
                    {versionInfo().version}
                  </p>
                  <Show when={versionInfo().edition}>
                    <div
                      class="about-field"
                      innerHTML={versionInfo().edition}
                      onClick={(e) => {
                        const a = e.target.closest("a");
                        if (a) e.preventDefault();
                      }}
                    />
                  </Show>
                </div>
              </section>

              <Show when={addonInfo()?.installed}>
                <div class="about-separator" />
                <section class="about-approw">
                  <div class="about-logo">
                    <svg class="ver-logo">
                      <use href="#kit-logo-light" />
                      <use href="#kit-logo-dark" />
                    </svg>
                  </div>
                  <div class="about-appinfo">
                    <p class="about-appname">{addonInfo().name}</p>
                    <p class="about-version">{t("strVersion", "Version")} {addonInfo().version}</p>
                    <Show when={addonInfo().license?.url}>
                      <p class="about-field about-license" style={{margin:0}}>
                        {t("strLicensedUnder")}{" "}
                        <a class="about-link" href={addonInfo().license.url} target="_blank" rel="noreferrer">
                          {addonInfo().license.title ?? addonInfo().license.url}
                        </a>
                      </p>
                    </Show>
                  </div>
                </section>
              </Show>

              <Show when={updatesOn()}>
                <div class="about-separator" />
              </Show>

              <Show when={updatesOn()}>
                <section class="about-updates">
                  <Show when={updateStatus()}>
                    <div class="update-status-row">
                      <svg class="icon update-icon" data-icon={updateStatus().icon}>
                        <use href={`#${updateStatus().icon ?? "success"}`} />
                      </svg>
                      <span class="update-status-text">{updateStatus().text}</span>
                    </div>
                    <Show when={updateStatus().button?.text}>
                      <div class="update-action-row">
                        <Button variant="accent" onClick={execUpdateAction} disabled={updateStatus().button.lock === "true"}>
                          {updateStatus().button.text}
                        </Button>
                      </div>
                    </Show>
                  </Show>
                </section>
              </Show>

              <div class="about-copyright">
                <Show when={versionInfo().changelog}>
                  <div class="about-field">
                    <a class="about-link" href={versionInfo().changelog} target="_blank" rel="noreferrer">
                      {t("aboutChangelog")}
                    </a>
                  </div>
                </Show>
                <div class="about-footer-row">
                  <Show when={versionInfo().rights}>
                    <div class="about-field about-rights">{versionInfo().rights}</div>
                  </Show>
                  <Show when={versionInfo().link}>
                    <a class="about-link about-field" href={versionInfo().link} target="_blank" rel="noreferrer">
                      {versionInfo().site}
                    </a>
                  </Show>
                </div>
              </div>
            </div>
          </Show>
        </div>
      </Dialog.Content>
    </Dialog>
  );
}
