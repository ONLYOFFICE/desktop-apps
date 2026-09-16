import { For } from 'solid-js';
import styles from './Section.module.css';
import Card from './Card';

export default function Section(props) {
  return (
    <section class={styles.section}>
      <h2 class={styles.title}>{props.title}</h2>
      <div class={styles.cards}>
        <For each={props.cards}>
          {(card) => <Card icon={card.icon || props.icon}
                          title={card.title}
                          description={card.description}
                          prompt={card.prompt}
                          usertext={card.usertext} />}
        </For>
      </div>
    </section>
  );
}
