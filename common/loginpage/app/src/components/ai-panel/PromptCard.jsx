import styles from "./AiPanel.module.css";

export function PromptCard(props) {
  return (
    <button type="button" class={styles.promptCard} onClick={() => props.onPick(props.text)}>
      <span class={styles.promptCardText}>{props.text}</span>
    </button>
  );
}
