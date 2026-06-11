import { For, Show } from "solid-js";
import { t } from "@/services/l10n";
import { PromptCard } from "./PromptCard.jsx";
import styles from "./AiPanel.module.css";

const promptKeys = ["aiChatPrompt1", "aiChatPrompt2", "aiChatPrompt3", "aiChatPrompt4"];

export function AiWelcome(props) {
  const showPrompts = () => !props.promptsClosed;

  return (
    <div class={styles.welcome} classList={{ [styles.welcomeCentered]: !showPrompts() }}>
      <div class={styles.hero}>
        <svg class="icon">
          <use href="#ai-docs" />
        </svg>
        <p class={styles.heading}>{t("aiChatWelcomeHeading")}</p>
      </div>

      <Show when={showPrompts()}>
        <div class={styles.prompts}>
          <For each={promptKeys}>{(key) => <PromptCard text={t(key)} onPick={props.onPick} />}</For>
          <div class={styles.promptsTools}>
            <button type="button" class={styles.promptsToolButton} onClick={() => props.onClosePrompts()}>
              {t("aiChatDismissPrompts")}
            </button>
          </div>
        </div>
      </Show>
    </div>
  );
}
