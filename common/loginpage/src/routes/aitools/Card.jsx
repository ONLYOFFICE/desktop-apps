import styles from './Card.module.css';
import { showToast } from '@/components/toast/toastutil';
import { sendPluginVisibility, sendTextToChat } from '@/services/ai-panel';
import { aichat, openSidePanel } from '@/stores/aichat';

export default function Card(props) {
  const handleClick = () => {
    if (props.prompt) {
      sendTextToChat(props.prompt);
      // sendPluginVisibility("expanded");
      if (aichat.expanded)
        window.location.hash = "#/aichat";
      else openSidePanel();
    } else {
      showToast('the feature is under construction');
    }
  };

  return (
    <button type="button" class={styles.card} onClick={handleClick}>
      <img src={props.icon} alt="" class={styles.icon} />
      <div class={styles.text}>
        <div class={styles.title}>{props.title}</div>
        <div class={styles.description}>{props.description}</div>
      </div>
    </button>
  );
}
