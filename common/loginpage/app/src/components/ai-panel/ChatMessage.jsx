import { Show } from "solid-js";
import { mergeClass } from "@/utils/classes";
import { ChatMessageActions } from "./ChatMessageActions.jsx";
import styles from "./AiPanel.module.css";

export function ChatMessage(props) {
  const isAssistant = () => props.message.role === "assistant";

  return (
    <div classList={mergeClass(styles.message, isAssistant() ? styles.messageAssistant : styles.messageUser)}>
      {props.message.text}
      <Show when={props.message.pending}>
        <span class={styles.caret}>▋</span>
      </Show>
      <Show when={isAssistant() && !props.message.pending && props.message.text}>
        <ChatMessageActions message={props.message} onRegenerate={props.onRegenerate} />
      </Show>
    </div>
  );
}
