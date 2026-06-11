// Chat-level actions. copy() is real (clipboard). The export/prompt/import handlers
// need native or backend support that does not exist yet — they are intentionally
// no-ops with TODO markers, mirroring the services/ai-chat.js inference seam.

export async function copy(text) {
  try {
    await navigator.clipboard.writeText(text ?? "");
  } catch {
    // clipboard unavailable (insecure context / denied) — swallow; UI stays unchanged
  }
}

export function exportChat(chatId) {
  // TODO(018+): export the conversation to a DOCX file via the native editor bridge
  void chatId;
}

export function turnIntoPrompt(chatId) {
  // TODO(018+): persist the conversation as a reusable prompt
  void chatId;
}

export function importChat(file) {
  // TODO(018+): import a conversation from a file
  void file;
}
