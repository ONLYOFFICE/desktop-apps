import { For, createEffect, on } from "solid-js";
import { currentMessages } from "@/stores/aichat";
import { ChatMessage } from "./ChatMessage.jsx";
import styles from "./AiPanel.module.css";

export function ChatThread(props) {
  let listRef;

  createEffect(
    on(
      () => [currentMessages().length, currentMessages().at(-1)?.text],
      () => {
        if (listRef) listRef.scrollTop = listRef.scrollHeight;
      },
    ),
  );

  return (
    <div ref={(el) => (listRef = el)} class={styles.thread}>
      <For each={currentMessages()}>{(message) => <ChatMessage message={message} onRegenerate={props.onRegenerate} />}</For>
    </div>
  );
}
