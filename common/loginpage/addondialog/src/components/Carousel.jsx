import { createSignal, For, onCleanup } from "solid-js";
import { t } from "@/services/l10n";
import slide1 from "../../assets/img/courusel1.svg";
import slide2 from "../../assets/img/courusel2.svg";
import slide3 from "../../assets/img/courusel3.svg";
import slide4 from "../../assets/img/courusel4.svg";
import slide5 from "../../assets/img/courusel5.svg";
import slide6 from "../../assets/img/courusel6.svg";
import slide1dark from "../../assets/img/courusel1-dark.svg";
import slide2dark from "../../assets/img/courusel2-dark.svg";
import slide3dark from "../../assets/img/courusel3-dark.svg";
import slide4dark from "../../assets/img/courusel4-dark.svg";
import slide5dark from "../../assets/img/courusel5-dark.svg";
import slide6dark from "../../assets/img/courusel6-dark.svg";
import styles from "./Carousel.module.css";

const SLIDES = [
  { image: slide1, dark: slide1dark, key: "addonSlide1" },
  { image: slide2, dark: slide2dark, key: "addonSlide2" },
  { image: slide3, dark: slide3dark, key: "addonSlide3" },
  { image: slide4, dark: slide4dark, key: "addonSlide4" },
  { image: slide5, dark: slide5dark, key: "addonSlide5" },
  { image: slide6, dark: slide6dark, key: "addonSlide6" },
];

export function Carousel() {
  const [index, setIndex] = createSignal(0);
  const [isDark, setIsDark] = createSignal(document.body.classList.contains("theme-type-dark"));

  const observer = new MutationObserver(() => {
    setIsDark(document.body.classList.contains("theme-type-dark"));
  });
  observer.observe(document.body, { attributes: true, attributeFilter: ["class"] });
  onCleanup(() => observer.disconnect());

  const prev = () => setIndex((i) => (i - 1 + SLIDES.length) % SLIDES.length);
  const next = () => setIndex((i) => (i + 1) % SLIDES.length);

  return (
    <div class={styles.carousel}>
      <div class={styles.imageRow}>
        <button type="button" class={styles.navBtn} onClick={prev} aria-label={t("addonPrevSlide", "Previous")}>
          <svg class={styles.navIcon}>
            <use href="#chevron-left" />
          </svg>
        </button>

        <img
          class={styles.image}
          src={isDark() ? SLIDES[index()].dark : SLIDES[index()].image}
          width="372"
          height="291"
          alt=""
          onClick={next}
        />

        <button type="button" class={styles.navBtn} onClick={next} aria-label={t("addonNextSlide", "Next")}>
          <svg class={styles.navIcon}>
            <use href="#chevron-right" />
          </svg>
        </button>
      </div>

      <div class={styles.dots}>
        <For each={SLIDES}>
          {(_, i) => (
            <span
              class={styles.dot}
              classList={{ [styles.dotActive]: i() === index() }}
              onClick={() => setIndex(i())}
            />
          )}
        </For>
      </div>

      <p class={styles.caption}>{t(SLIDES[index()].key)}</p>
    </div>
  );
}
