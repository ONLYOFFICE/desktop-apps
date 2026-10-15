import { Show } from 'solid-js';
import { Loader } from '@/components';
import styles from './index.module.css';
import { urlAiTools } from '@/services/ai-panel';

export default function AiToolsPanel() {
  return (
    <div class={styles.panel}>
      <Show when={urlAiTools()} fallback={<Loader />}>
        <iframe class={styles.frame} src={urlAiTools()} title="AI Tools" />
      </Show>
    </div>
  );
}
