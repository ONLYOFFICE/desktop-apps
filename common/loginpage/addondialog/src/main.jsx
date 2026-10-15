import { render } from "solid-js/web";
import App from "./App.jsx";
import { initLocale } from "./services/l10n";
import { initTheme } from "./services/theme";
import iconsSprite from "../assets/img/icons.svg?raw";
import "./styles/main.css";
import "./styles/themes.css";

document.body.insertAdjacentHTML("afterbegin", iconsSprite);

initLocale();
initTheme();

render(() => <App />, document.getElementById("root"));
