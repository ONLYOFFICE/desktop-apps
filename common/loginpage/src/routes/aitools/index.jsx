import { For, Show, createMemo } from 'solid-js';
import styles from './index.module.css';
import Section from './Section';
import Toast from '@/components/toast/Toast';
import { buildTools } from './tools';
import { urlAiTools } from '@/services/ai-panel';

export default function AiToolsPanel() {
  const tools = createMemo(buildTools);

  return (
    <div class={styles.panel}>
      <Show
        when={!urlAiTools()}
        fallback={<iframe class={styles.frame} src={urlAiTools()} title="AI Tools" />}
      >
        <div class={styles.content}>
          <h1 class={styles.header}>AI Tools</h1>
          <div class={styles.sections}>
            <For each={tools()}>
              {(block) => <Section title={block.title} icon={block.icon} cards={block.cards} />}
            </For>
          </div>
        </div>
      </Show>
      <Toast />
    </div>
  );
}
