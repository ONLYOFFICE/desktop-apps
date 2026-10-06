import { Show, createMemo } from "solid-js";
import { t } from "@/services/l10n.js";
import { aboutOpen, closeAbout, versionInfo, updatesOn, updateStatus, execUpdateAction, addonInfo } from "@/services/about.js";
import { Dialog, Button } from "@/components";
import "./about.css";
import { command } from "@/sdk/bridge";

function parseLicenseInfo(html) {
  if (!html) return null;
  const anchorMatch = html.match(/<a\s([\s\S]*?)>([\s\S]*?)<\/a>/i);
  if (!anchorMatch) return { prefix: html.replace(/<[^>]+>/g, "").trim(), url: null, linkText: null };

  const attrStr = anchorMatch[1];
  const linkText = anchorMatch[2].replace(/<[^>]+>/g, "").trim();
  const prefix = html.slice(0, html.search(/<a\s/i)).replace(/<[^>]+>/g, "").trim();

  const onclickMatch = attrStr.match(/onclick=["']window\.open\(['"]([^'"]+)['"]\)/i);
  let url = onclickMatch ? onclickMatch[1] : null;
  if (!url) {
    const hrefMatch = attrStr.match(/href=["']([^"'#][^"']*)["']/i);
    url = hrefMatch ? hrefMatch[1] : null;
  }

  return { prefix, url, linkText };
}

export default function About() {
  const licenseInfo = createMemo(() =>
    versionInfo()?.edition ? parseLicenseInfo(versionInfo().edition) : null
  );

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
                  <svg class="ver-logo" viewBox="0 0 52 45">
                    <use href="#idx-logo"/>
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
                  <Show when={licenseInfo()}>
                    <div class="about-field about-license">
                      <Show when={licenseInfo().prefix}>
                        {licenseInfo().prefix}{" "}
                      </Show>
                      <Show when={licenseInfo().url}>
                        <a
                          class="about-link"
                          href="#"
                          onClick={(e) => { e.preventDefault(); command("open:license", licenseInfo().url); }}
                        >
                          {licenseInfo().linkText}
                        </a>
                      </Show>
                    </div>
                  </Show>
                </div>
              </section>

              <Show when={addonInfo()?.installed || addonInfo()?.integrated}>
                <div class="about-separator" />
                <section class="about-approw">
                  <div class="about-logo">
                    <svg class="ver-logo" viewBox="0 0 44 44">
                      <use href="#ai-tools-logo"/>
                    </svg>
                  </div>
                  <div class="about-appinfo">
                    <p class="about-appname">
                      {addonInfo().name}
                      <span class="about-beta-badge">Beta</span>
                    </p>
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
                    <Show when={updateStatus().button}>
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
