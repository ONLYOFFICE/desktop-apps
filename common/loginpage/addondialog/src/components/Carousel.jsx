import { createSignal, For } from "solid-js";
import { t } from "@/services/l10n";
import slide1 from "../../assets/img/courusel1.svg";
import slide2 from "../../assets/img/courusel2.svg";
import slide3 from "../../assets/img/courusel3.svg";
import slide4 from "../../assets/img/courusel4.svg";
import slide5 from "../../assets/img/courusel5.svg";
import slide6 from "../../assets/img/courusel6.svg";
import styles from "./Carousel.module.css";

const SLIDES = [
  { image: slide1, key: "addonSlide1" },
  { image: slide2, key: "addonSlide2" },
  { image: slide3, key: "addonSlide3" },
  { image: slide4, key: "addonSlide4" },
  { image: slide5, key: "addonSlide5" },
  { image: slide6, key: "addonSlide6" },
];

export function Carousel() {
  const [index, setIndex] = createSignal(0);

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

        <img class={styles.image} src={SLIDES[index()].image} width="372" height="291" alt="" />

        <button type="button" class={styles.navBtn} onClick={next} aria-label={t("addonNextSlide", "Next")}>
          <svg class={styles.navIcon}>
            <use href="#chevron-right" />
          </svg>
        </button>
      </div>

      <div class={styles.dots}>
        <For each={SLIDES}>
          {(_, i) => <span class={styles.dot} classList={{ [styles.dotActive]: i() === index() }} />}
        </For>
      </div>

      <p class={styles.caption}>{t(SLIDES[index()].key)}</p>
    </div>
  );
}
