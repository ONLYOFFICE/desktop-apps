import { createSignal } from "solid-js";

export const promoButtons = [
    {
        icon: '#about',
        id: 'developer',
        text: 'Docs Developer',
        url: "https://www.onlyoffice.com/developer-edition",
    },
    {
        icon: '#about',
        id: 'enterprise',
        text: 'Docs Enterprise',
        url: "https://www.onlyoffice.com/docs-enterprise",
    },
    {
        icon: '#about',
        id: 'docspace',
        text: 'DocSpace',
        url: "https://www.onlyoffice.com/docspace-developer",
    },
];

const [promoVisible, setPromoVisible] = createSignal(true);
export { promoVisible };

export function openPromo(id) {
    const obj = promoButtons.find(b => b.id === id);
    if (obj)
        window.open(obj.url, '_blank');
}

