import { langCode } from '@/services/l10n';
import { groups, iconRegistry } from './groups';

const cardModules = import.meta.glob('./cards/*.json', { eager: true });
const allCards = Object.values(cardModules).map((m) => m.default);

function localize(card, lang) {
  return card[lang] ?? card[lang?.split('_')[0]] ?? card.en ?? {};
}

export function buildTools() {
  const lang = typeof langCode() === 'string' ? langCode() : 'en';

  const grouped = {};
  for (const card of allCards) {
    const g = card.group;
    if (!grouped[g]) grouped[g] = [];
    const loc = localize(card, lang);
    grouped[g].push({
      id: card.id,
      title: loc.title ?? card.id,
      description: loc.description ?? '',
      prompt: card.prompt ?? '',
      icon: card.icon,
    });
  }

  return groups
    .map((g) => ({
      ...g,
      cards: (grouped[g.id] ?? []).map((c) => ({ ...c, icon: iconRegistry[c.icon] ?? g.icon })),
    }))
    .filter((g) => g.cards.length > 0);
}
