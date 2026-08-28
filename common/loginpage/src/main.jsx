import { render } from "solid-js/web";
import App from "./app.jsx";
import "../noconnect/styles.css";
import "./services/noconnect-init.js";
import { initLocale } from "./services/l10n";
import { initDisplay } from "./services/display";
import { onNativeMessage, dropOfficeFiles } from "./sdk";
import "./styles/sprites/toolicons@1.25x.css";
import "./styles/sprites/toolicons@1.5x.css";
import "./styles/sprites/toolicons@1.75x.css";
import "./styles/sprites/toolicons20@1.25x.css";
import "./styles/sprites/toolicons20@1.5x.css";
import "./styles/sprites/toolicons20@1.75x.css";
import "./styles/sprites/formats@1.25x.css";
import "./styles/sprites/formats@1.5x.css";
import "./styles/sprites/formats@1.75x.css";

initLocale();
initDisplay();

const dispose = render(() => <App />, document.getElementById("root"));

document.getElementById("wrap").ondrop = function (e) {
  dropOfficeFiles();
  e.preventDefault();
  return false;
};

document.getElementById("wrap").ondragover = function (e) {
  e.dataTransfer.dropEffect = "copy";
  e.preventDefault();
  return false;
};

onNativeMessage((cmd) => {
  if (cmd === "app:ready") {
    const mask = document.getElementById("loading-mask");
    if (mask) mask.style.display = "none";
  }
});

if (import.meta.hot) {
  // import.meta.hot.accept();
  // import.meta.hot.dispose(() => dispose());

  window.addEventListener('keydown', (event) => {
    // Check for Alt + R (or customize your preferred keys)
    if (event.altKey && event.key.toLowerCase() === 'r') {
      event.preventDefault();

      // Use Vite's WebSocket to tell the server to do a full reload
      import.meta.hot.send('vite:invalidate', { path: '*' });

      // Fallback: Force a hard location reload if HMR is stuck
      window.location.reload();
    }
  });
}
