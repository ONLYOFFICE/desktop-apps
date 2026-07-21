import { render } from "solid-js/web";
import App from "./App.jsx";
import { initLocale } from "./services/l10n";
import iconsSprite from "../assets/img/icons.svg?raw";
import "./styles/main.css";

document.body.insertAdjacentHTML("afterbegin", iconsSprite);

initLocale();

render(() => <App />, document.getElementById("root"));
