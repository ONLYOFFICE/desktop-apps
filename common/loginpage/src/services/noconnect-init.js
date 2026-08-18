import noconnectCode from "../../noconnect/code.js?raw";

const s = document.createElement("script");
s.textContent = noconnectCode;
document.head.appendChild(s);

if (import.meta.env.DEV) {
  import("../../noconnect/l10n/ru.js?raw").then(({ default: code }) => {
    const s2 = document.createElement("script");
    s2.textContent = code;
    document.head.appendChild(s2);
  });
}
