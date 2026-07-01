// import styles from "../styles/promo.module.css";
import "../styles/promo.css";
import { settings } from "@/stores/settings";

export const promoButtons = [
    {
        icon: '#promo1',
        id: 'developer',
        // cls: `${styles.button.promoBtn} ${styles.promoBtnVar1}`,
        text: 'Docs Developer',
        url: "https://www.onlyoffice.com/developer-edition",
    },
    {
        icon: '#promo2',
        id: 'enterprise',
        cls: 'promo-btn promo-btn-var2',
        text: 'Docs Enterprise',
        url: "https://www.onlyoffice.com/docs-enterprise",
    },
    {
        icon: '#promo2',
        id: 'docspace',
        cls: 'promo-btn promo-btn-var3',
        text: 'DocSpace',
        url: "https://www.onlyoffice.com/docspace-developer",
    },
];

export const promoVisible = () => /*settings.promoButtons !==*/ false;

export function openPromo(id) {
    const obj = promoButtons.find(b => b.id === id);
    if (obj)
        window.open(obj.url, '_blank');
}

