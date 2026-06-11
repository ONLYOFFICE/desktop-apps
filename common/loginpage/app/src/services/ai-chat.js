// Stub inference seam — the ONLY place chat replies originate. A later plan swaps this
// implementation (real OpenAI-compatible client or the native `panel:external` host)
// without touching any UI component. Keep it self-contained: no network, no secrets.

const CANNED =
  "This is a stubbed assistant reply. Real inference is not wired up yet — the panel only " +
  "exercises the streaming UI. Swap services/ai-chat.js to connect a real model.";

const delay = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

function chunkWords(text, size) {
  const words = text.split(" ");
  const chunks = [];
  for (let i = 0; i < words.length; i += size) chunks.push(words.slice(i, i + size).join(" "));
  return chunks;
}

export async function* sendMessage(text, { model } = {}) {
  void model;
  const asked = text?.trim();
  const reply = asked ? `${CANNED}\n\nYou asked: “${asked}”` : CANNED;
  const chunks = chunkWords(reply, 5);
  for (let i = 0; i < chunks.length; i++) {
    await delay(110);
    yield i === 0 ? chunks[i] : ` ${chunks[i]}`;
  }
}
