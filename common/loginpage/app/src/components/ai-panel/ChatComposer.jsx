import { createEffect, on } from "solid-js";
import { Select } from "@/components";
import { t } from "@/services/l10n";
import { aichat, currentDraft, setAichat, setDraft } from "@/stores/aichat";
import styles from "./AiPanel.module.css";

const MAX_INPUT_HEIGHT = 180;

const MODEL_OPTIONS = [
  { value: "openai-gpt", label: "Open AI – GPT 5.3" },
  { value: "openai-gpt-mini", label: "Open AI – GPT 5.3 mini" },
];

export function ChatComposer(props) {
  let textareaRef;

  function autoGrow() {
    if (!textareaRef) return;
    textareaRef.style.height = "auto";
    textareaRef.style.height = `${Math.min(textareaRef.scrollHeight, MAX_INPUT_HEIGHT)}px`;
  }

  createEffect(on(currentDraft, autoGrow));

  const canSend = () => currentDraft().trim().length > 0;

  // TODO(018+): attachments and slash-commands are a later plan
  const onAttach = () => {};
  const onCommand = () => {};

  function onKeyDown(e) {
    if (e.key === "Enter" && !e.shiftKey) {
      e.preventDefault();
      props.onSend();
    }
  }

  return (
    <div class={styles.composer}>
      <div class={styles.composerBox}>
        <textarea
          ref={(el) => (textareaRef = el)}
          class={styles.textarea}
          rows="1"
          placeholder={t("aiChatPlaceholder")}
          value={currentDraft()}
          onInput={(e) => setDraft(e.currentTarget.value)}
          onKeyDown={onKeyDown}
        />
        <div class={styles.composerControls}>
          <button type="button" class={styles.composerIconButton} aria-label={t("aiChatAttach")} onClick={onAttach}>
            <svg class={styles.iconMd}>
              <use href="#ai-plus" />
            </svg>
          </button>
          <button type="button" class={styles.composerIconButton} aria-label={t("aiChatCommand")} onClick={onCommand}>
            <svg class={styles.iconMd}>
              <use href="#ai-command" />
            </svg>
          </button>
          <div class={styles.spacer} />
          <Select options={MODEL_OPTIONS} value={aichat.model} onChange={(v) => setAichat("model", v)} disallowEmptySelection>
            <Select.Trigger class={styles.modelTrigger} />
            <Select.Content />
          </Select>
          <button type="button" class={styles.sendButton} disabled={!canSend()} aria-label={t("aiChatSend")} onClick={() => props.onSend()}>
            <svg class={styles.smallIcon}>
              <use href="#ai-send" />
            </svg>
          </button>
        </div>
      </div>
      <p class={styles.disclaimer}>{t("aiChatDisclaimer")}</p>
    </div>
  );
}
