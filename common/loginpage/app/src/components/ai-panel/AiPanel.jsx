import { Show, createSignal } from "solid-js";
import {
  aichat,
  appendMessage,
  appendToMessage,
  currentDraft,
  currentMessages,
  newChat,
  resetMessage,
  setDraft,
  setMessagePending,
} from "@/stores/aichat";
import { sendMessage } from "@/services/ai-chat";
import { AiPanelHeader } from "./AiPanelHeader.jsx";
import { AiWelcome } from "./AiWelcome.jsx";
import { ChatComposer } from "./ChatComposer.jsx";
import { ChatThread } from "./ChatThread.jsx";
import { HistoryPanel } from "./HistoryPanel.jsx";
import { ResizeHandle } from "./ResizeHandle.jsx";
import styles from "./AiPanel.module.css";

export function AiPanel() {
  const [promptsClosed, setPromptsClosed] = createSignal(false);
  const hasMessages = () => currentMessages().length > 0;

  function onNewChat() {
    newChat();
    setPromptsClosed(false);
  }

  async function streamReply(prompt, replyId) {
    try {
      for await (const chunk of sendMessage(prompt, { model: aichat.model })) appendToMessage(replyId, chunk);
    } finally {
      setMessagePending(replyId, false);
    }
  }

  async function onSend() {
    const text = currentDraft().trim();
    if (!text) return;
    appendMessage("user", text);
    setDraft("");
    const replyId = appendMessage("assistant", "", true);
    await streamReply(text, replyId);
  }

  async function onRegenerate(messageId) {
    const msgs = currentMessages();
    const idx = msgs.findIndex((m) => m.id === messageId);
    if (idx < 0) return;
    const prompt = msgs
      .slice(0, idx)
      .reverse()
      .find((m) => m.role === "user")?.text;
    if (!prompt) return;
    resetMessage(messageId, true);
    await streamReply(prompt, messageId);
  }

  return (
    <div class={styles.panel}>
      <ResizeHandle />
      <Show
        when={aichat.panelView === "history"}
        fallback={
          <>
            <AiPanelHeader onNewChat={onNewChat} />
            <div class={styles.body}>
              <Show
                when={hasMessages()}
                fallback={
                  <AiWelcome
                    onPick={(text) => setDraft(text)}
                    promptsClosed={promptsClosed()}
                    onClosePrompts={() => setPromptsClosed(true)}
                  />
                }
              >
                <ChatThread onRegenerate={onRegenerate} />
              </Show>
            </div>
            <ChatComposer onSend={onSend} />
          </>
        }
      >
        <HistoryPanel />
      </Show>
    </div>
  );
}
