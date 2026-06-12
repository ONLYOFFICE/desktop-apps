# Rule: icons are sprite injections, never JSX components

**Status: MANDATORY. No exceptions without an owner sign-off recorded in this file.**

## The rule

**Never create a file that defines icons as inline-SVG JSX components.**

This forbids — by intent, not just by name:

- `icons.jsx` / `icons.tsx` / `Icons.jsx`
- `*-icons.jsx`, `*Icon.jsx`, `*Icons.jsx` (e.g. `ai-icons.jsx`, `ChatIcons.jsx`)
- any module exporting components like `IconClose`, `IconSend`, `Glyph`, etc. whose
  body is hard-coded `<svg>…<path d="…" />…</svg>` markup
- any component that ships icon geometry (`<path>`, `<circle>`, `viewBox` over glyph
  artwork) inside `.jsx`

If you are about to write `<path d=` inside a `.jsx` file to draw an icon, **stop** —
you are violating this rule.

## Why

Icons in this app are **build-time sprite injections**, not React/Solid components:

1. Source artwork lives as `<symbol id="…">` sets in `../res/img/generated/*.svg`
   (`aiicons.svg`, `toolicons.svg`, `common.svg`, …).
2. `vite.config.js` → `injectSprites()` concatenates those sprites into the HTML
   `<body>` (before `<div id="root">`) at build and dev time.
3. Markup references a symbol by id:

   ```jsx
   <svg class="icon">
     <use href="#home" />
   </svg>
   ```

Defining glyphs as JSX `<path>` components **duplicates artwork that already exists in
the sprite**, bypasses the injection pipeline, breaks single-source-of-truth for icons,
desyncs from native/legacy icon sets, and bloats the bundle. The deleted
`src/components/ai-panel/icons.jsx` had re-drawn `ai-close`, `ai-history`, `ai-plus`,
`ai-command`, `ai-send`, `ai-chevron` — all of which **already exist** as symbols in
`aiicons.svg`. It was removed in commit `8ff34911e`. Do not bring it back.

## The only correct pattern

Reference an existing sprite symbol by id:

```jsx
<svg class="icon">
  <use href="#aichat" />
</svg>
```

Use the `icon` class (sidebar tool icons) or a CSS-module class sized for the call site
(e.g. `styles.pillIcon` in `AiPanel.module.css`). `currentColor` flows from the host;
size and color belong in CSS, never in JSX glyph markup.

## Need an icon that does not exist yet?

Add it to the sprite source, not to a `.jsx` file:

1. Add/extend a `<symbol id="your-id" viewBox="…">…</symbol>` in the appropriate
   `../res/img/generated/*.svg` (or its upstream source that regenerates it).
2. Reference it with `<use href="#your-id" />`.

If the sprite cannot be regenerated in your task, ask the owner — do **not** work around
it by inlining the glyph in a component.

## Precedent

- `8ff34911e` — `refactor(ai): remove orphaned ai-panel icons.jsx` (deleted the 82-line
  inline-SVG glyph module). This rule exists to prevent its recurrence.
