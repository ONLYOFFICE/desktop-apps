import { createStore, produce } from "solid-js/store";
import { makePersisted } from "@solid-primitives/storage";

export const AI_PANEL_DEFAULT_WIDTH = 400;
export const AI_PANEL_MIN_WIDTH = 320;
export const AI_CONTENT_MIN_WIDTH = 800;

const DAY = 86400000;

let seq = 0;
const newId = (prefix = "m") => `${prefix}${Date.now()}-${seq++}`;

function makeChat() {
  const now = Date.now();
  return { id: newId("c"), title: "", messages: [], draft: "", createdAt: now, updatedAt: now };
}

function deriveTitle(text) {
  const clean = text.trim().replace(/\s+/g, " ");
  if (!clean) return "";
  return clean.length > 40 ? `${clean.slice(0, 40)}…` : clean;
}

const seed = makeChat();

const defaultAichat = {
  open: true,
  width: AI_PANEL_DEFAULT_WIDTH,
  model: "openai-gpt",
  chats: { [seed.id]: seed },
  order: [seed.id],
  currentId: seed.id,
  panelView: "chat",
  query: "",
};

const persistedKeys = ["open", "width", "model", "chats", "order", "currentId"];

function hydrate(raw) {
  const saved = JSON.parse(raw);
  const valid = saved.chats && Array.isArray(saved.order) && saved.order.length && saved.chats[saved.currentId];
  if (!valid) {
    const fresh = makeChat();
    return { ...defaultAichat, chats: { [fresh.id]: fresh }, order: [fresh.id], currentId: fresh.id };
  }
  return { ...defaultAichat, ...saved, panelView: "chat", query: "" };
}

const [aichat, setAichat] = makePersisted(createStore({ ...defaultAichat }), {
  name: "aichat",
  serialize: (state) => JSON.stringify(Object.fromEntries(persistedKeys.map((k) => [k, state[k]]))),
  deserialize: hydrate,
});

export const currentChat = () => aichat.chats[aichat.currentId];
export const currentMessages = () => currentChat()?.messages ?? [];
export const currentDraft = () => currentChat()?.draft ?? "";

export function setDraft(text) {
  setAichat("chats", aichat.currentId, "draft", text);
}

function touch(id) {
  setAichat("chats", id, "updatedAt", Date.now());
  setAichat("order", (order) => [id, ...order.filter((x) => x !== id)]);
}

export function appendMessage(role, text, pending = false) {
  const id = newId();
  const cid = aichat.currentId;
  setAichat("chats", cid, "messages", (prev) => [...prev, { id, role, text, pending }]);
  if (role === "user" && !currentChat().title) setAichat("chats", cid, "title", deriveTitle(text));
  touch(cid);
  return id;
}

export function appendToMessage(messageId, chunk) {
  setAichat(
    "chats",
    aichat.currentId,
    "messages",
    (m) => m.id === messageId,
    "text",
    (prev) => prev + chunk,
  );
}

export function setMessagePending(messageId, pending) {
  setAichat("chats", aichat.currentId, "messages", (m) => m.id === messageId, "pending", pending);
}

export function resetMessage(messageId, pending = true) {
  setAichat(
    "chats",
    aichat.currentId,
    "messages",
    (m) => m.id === messageId,
    produce((m) => {
      m.text = "";
      m.pending = pending;
    }),
  );
}

export function newChat() {
  const chat = makeChat();
  setAichat(
    produce((s) => {
      s.chats[chat.id] = chat;
      s.order = [chat.id, ...s.order];
      s.currentId = chat.id;
      s.panelView = "chat";
    }),
  );
}

export function selectChat(id) {
  if (!aichat.chats[id]) return;
  setAichat("currentId", id);
  setAichat("panelView", "chat");
}

export function deleteChat(id) {
  setAichat(
    produce((s) => {
      delete s.chats[id];
      s.order = s.order.filter((x) => x !== id);
      if (s.currentId === id) {
        if (s.order.length) {
          s.currentId = s.order[0];
        } else {
          const fresh = makeChat();
          s.chats[fresh.id] = fresh;
          s.order = [fresh.id];
          s.currentId = fresh.id;
        }
      }
    }),
  );
}

export function renameChat(id, title) {
  const clean = title.trim();
  if (clean) setAichat("chats", id, "title", clean);
}

export function clearAllChats() {
  setAichat(
    produce((s) => {
      const fresh = makeChat();
      s.chats = { [fresh.id]: fresh };
      s.order = [fresh.id];
      s.currentId = fresh.id;
      s.panelView = "chat";
    }),
  );
}

export function setView(view) {
  setAichat("panelView", view);
}

export function setQuery(query) {
  setAichat("query", query);
}

function bucketOf(ts, now) {
  const startToday = new Date(now).setHours(0, 0, 0, 0);
  if (ts >= startToday) return "today";
  if (ts >= startToday - DAY) return "yesterday";
  if (ts >= startToday - 7 * DAY) return "prev7";
  if (ts >= startToday - 30 * DAY) return "prev30";
  return "older";
}

const BUCKET_ORDER = ["today", "yesterday", "prev7", "prev30", "older"];
const BUCKET_LABEL = {
  today: "aiChatHistToday",
  yesterday: "aiChatHistYesterday",
  prev7: "aiChatHistPrev7",
  prev30: "aiChatHistPrev30",
  older: "aiChatHistOlder",
};

export function groupedHistory() {
  const now = Date.now();
  const q = aichat.query.trim().toLowerCase();
  const groups = {};
  for (const id of aichat.order) {
    const chat = aichat.chats[id];
    if (!chat || (!chat.messages.length && !chat.title)) continue;
    const title = chat.title || "New chat";
    if (q && !title.toLowerCase().includes(q)) continue;
    (groups[bucketOf(chat.updatedAt, now)] ??= []).push(chat);
  }
  return BUCKET_ORDER.filter((b) => groups[b]?.length).map((b) => ({ bucket: b, labelKey: BUCKET_LABEL[b], chats: groups[b] }));
}

export { aichat, setAichat, defaultAichat };
