# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

`common/loginpage` is a SolidJS rewrite of the ONLYOFFICE desktop app's "start page"
(recents, templates, AI tools, cloud connections, settings). It runs inside a CEF
webview where the native host injects `window.AscDesktopEditor` and
`window.RendererProcessVariable`. All app state that must survive/reflect native state
(settings, recents, theme, locale) flows through the bridge in `src/sdk/`.

This directory actually contains **three independent Vite apps**, each with its own
`package.json`/`vite.config.js` and each built as a single-file HTML bundle
(`vite-plugin-singlefile`) so it can be loaded directly by the CEF shell:

| App                       | Root           | Purpose                                                              |
| ------------------------- | -------------- | --------------------------------------------------------------------|
| main loginpage app        | `src/`         | The primary start-page UI (this is what most tasks touch)           |
| `addondialog/`            | `addondialog/` | Standalone dialog shown when installing/removing the AI Tools addon |
| `noconnect/`              | `noconnect/`   | Standalone "no connection" / connection-error page                  |

`providers/` holds per-cloud-provider JSON configs (`box`, `dropbox`, `nextcloud`,
`onlyoffice`, `owncloud`, `seafile`, etc.) consumed by the Clouds panel — each has a
`config.json` (name, `check.url` availability endpoint, `editorPage`/`startPage`
matchers, icon assets) plus an `assets/` folder. `addon/externalcloud.json` is a
related addon-level manifest.

## Commands

Run from `common/loginpage/` unless noted:

| Command                             | Purpose                                                            |
| ------------------------------------| ------------------------------------------------------------------|
| `npm run dev`                       | Vite dev server on port 3000 for the main app; also writes dev HTML into `./deploy` |
| `npm run build`                     | Single-file bundle of the main app to `./deploy`                   |
| `npm run build:all`                 | Builds `noconnect` then the main app (order matters — main app inlines `deploy/noconnect.js`) |
| `npm run sprites`                   | Regenerate SVG/PNG icon sprite sheets (`node ./build/generate-sprites.js`) |
| `npm run lint` / `npm run lint:fix` | oxlint                                                              |
| `npm run fmt`                       | oxfmt (printWidth 140)                                              |

`addondialog/` and `noconnect/` are separate npm packages with their own `dev`/`build`
scripts (`cd addondialog && npm run build`, etc.) — run them from inside each directory.

There is no test runner configured in this repo (no `test` script, no test files).

## Package manager

npm. `package-lock.json` is tracked in each of the three app directories. Do not
introduce `bun.lock`.

## Main app layout (`src/`)

```
src/sdk/          native bridge (bridge.js installs pub/sub + command() on
                  AscDesktopEditor; events.js, files.js, recents.js, format.js, app.js)
src/services/     singletons (l10n.js, settings-sync.js — owns ALL native settings
                  traffic, ai-panel.js, portals.js, theme.js, about.js)
src/stores/       persisted Solid stores (settings.js, aichat.js, ext-plugins.js,
                  plugin-settings.js, plugin-modals.js)
src/routes/       pages (hash router; routes registered in src/app.jsx)
src/components/   Kobalte-wrapping UI kit (barrel: src/components/index.js)
src/shell/        app chrome (Sidebar, TopBar)
src/data/         static shared reference data (e.g. countries.js)
src/styles/       design tokens (tokens.css), sprites, per-theme CSS, global styles
src/utils/        small framework-free helpers (classes.js, html.js, ...)
```

`src/app.jsx` owns the `HashRouter` route table and the `Shell` layout (sidebar +
center panel + optional AI chat side/full column + plugin iframes). Panels that are
expensive or rarely used are lazy-loaded (`lazy(() => import(...))`); Home and
Templates are mounted eagerly/kept-alive and toggled via a `panel-active` class instead
of being routed, so their state survives navigation.

## Native bridge

- `src/sdk/bridge.js` wraps `window.AscDesktopEditor`: `command(name, param)` calls
  `execCommand`, and native fires window-level callbacks (`onupdaterecents`,
  `on_native_message`, `onPluginEvent`, etc.) that get re-dispatched through a small
  pub/sub (`sdk.on/remove/fire`).
- `src/services/settings-sync.js` is the single owner of settings traffic in both
  directions — do not read/write native settings from anywhere else.
- l10n tables are injected via `<script>` tags in `index.html`, filling `window.l10n`.
- Native-side handlers live outside this directory, in the sibling `win-linux` app:
  `win-linux/src/cascapplicationmanagerwrapper.cpp` (`applySettings`) and
  `win-linux/src/prop/cmainpanelimpl.cpp` (settings:init payload shape).

## Conventions

- Components wrap Kobalte primitives; compound API via `Object.assign(Root, { Sub, ... })`.
  Exemplar: `src/components/select/Select.jsx`.
- Styling: CSS Modules (`*.module.css`) for components; route-level plain `*.styles.css`
  for pages. Design tokens live in `src/styles/tokens.css` (theme classes on
  `documentElement`). CSS container queries (`@container`, `container-type: inline-size`)
  are used for component-local responsive breakpoints instead of viewport media queries
  where a panel's width can differ from the window's.
- Class merging: `mergeClass(base, extra)` from `src/utils/classes.js` with Solid
  `classList`. Never use clsx or template strings for class composition.
- **Never destructure Solid component props** — Solid reactivity breaks on
  destructuring. Read `props.foo` directly. The vendored skill at
  `.claude/skills/solid-js-best-practices/` is the style authority for all SolidJS
  patterns (signals vs. stores, effects, refs, lists, testing, etc.).
- **Sidebar rule (do not "fix")**: `src/shell/Sidebar.jsx` renders `<li>` directly
  under `<nav class="tool-menu">` with NO `<ul>` wrapper. The CSS in
  `Sidebar.styles.css` uses the `.tool-menu > .menu-item` direct-child selector. This
  is a deliberate owner decision — never add a `<ul>` wrapper.
- **Icons are sprite injections, never JSX** — render
  `<svg class="icon"><use href="#id" /></svg>` against the `<symbol>` sets injected
  from `./res/img/generated/*.svg` by `build/vite.config.js` (`injectSprites`). NEVER
  create an `icons.jsx` / `*-icons.jsx` / `Icon*.jsx` module or inline `<path>` glyph
  markup in a `.jsx` file. Full rule (mandatory, no exceptions without owner sign-off):
  `.claude/rules/icons.md`.
- AI Tools card content (`src/routes/aitools/cards/*.json`) is localized data, not
  code: each file has `id`, `group`, `prompt` (English-only, never translated) and a
  `locale` map keyed by locale code (matching `locale/*.js` filenames, e.g. `ru`,
  `pt-BR`, `zh-CN`) with `{ title, description, usertext }` per language.

## Build pipeline notes (`build/vite.config.js`)

- Custom plugins: `generateSvgSpritesPlugin`/`generatePngSpritesPlugin` (icon sprites),
  `lessColors` (transpiles legacy LESS `fade()`/`darken()`/`lighten()` calls found in
  CSS into plain `rgba()`/hex), `inlineExternalHtmlResources` (inlines `../`-relative
  `<script src>`/`<link>` tags from `index.html` at build time, and specifically
  substitutes the built `noconnect.js` from `../deploy`), `injectSprites`.
- Production build uses `vite-plugin-singlefile` — everything (JS, CSS, inlined
  assets up to 100MB) is emitted as one HTML file to `../deploy`, since the CEF host
  loads a single local file.

## Comment policy

No inline comments unless logic is non-obvious. Meaningful names over comments.
