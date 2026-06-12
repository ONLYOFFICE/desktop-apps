# ONLYOFFICE Desktop — loginpage app

SolidJS rewrite of the ONLYOFFICE desktop "start page" (`common/loginpage/app`), running
inside a CEF webview where native injects `window.AscDesktopEditor`. The legacy
jQuery implementation being replaced is `common/loginpage/src` — treat it as the
behavioral reference, never modify it from this app.

## Commands

Run from `common/loginpage/app/`:

| Command                             | Purpose                                                                       |
| ----------------------------------- | ----------------------------------------------------------------------------- |
| `npm run dev`                       | Vite dev server on port 3000; also writes dev HTML into `../build/dist-solid` |
| `npm run build`                     | Single-file bundle to `../build/dist-solid`                                   |
| `npm run lint` / `npm run lint:fix` | oxlint                                                                        |
| `npm run fmt`                       | oxfmt (printWidth 140)                                                        |

## Package manager

npm. `package-lock.json` is tracked in this directory (re-included via `.gitignore`
despite the repo-root rule). Do not introduce `bun.lock`.

## Layout

```
src/sdk/          native bridge (bridge.js installs pub/sub on AscDesktopEditor;
                  events.js, files.js, recents.js, format.js, app.js)
src/services/     singletons (l10n.js, settings-sync.js — owns ALL native settings traffic)
src/stores/       persisted Solid stores (settings.js)
src/routes/       pages (hash router; routes defined in src/app.jsx)
src/components/   Kobalte-wrapping UI kit (barrel: src/components/index.js)
src/shell/        app chrome (Sidebar, TopBar)
src/styles/       design tokens (tokens.css) and global styles
```

## Conventions

- Components wrap Kobalte primitives; compound API via `Object.assign(Root, { Sub, ... })`.
  Exemplar: `src/components/select/Select.jsx`.
- Styling: CSS Modules (`*.module.css`) for components; route-level plain `*.styles.css`
  for pages. Design tokens in `src/styles/tokens.css` (theme classes on `documentElement`).
- Class merging: `mergeClass(base, extra)` from `src/utils/classes.js` with Solid `classList`.
  Never use clsx or template strings for class composition.
- **Never destructure Solid component props** — Solid reactivity breaks on destructuring.
  Read `props.foo` directly. The vendored skill at `.claude/skills/solid-js-best-practices/`
  is the style authority.
- **Sidebar rule (do not "fix")**: `src/shell/Sidebar.jsx` renders `<li>` directly under
  `<nav class="tool-menu">` with NO `<ul>` wrapper. The CSS in `Sidebar.styles.css` uses
  the `.tool-menu > .menu-item` direct-child selector. This structure is a deliberate
  owner decision — never add a `<ul>` wrapper.
- **Icons are sprite injections, never JSX** — render `<svg class="icon"><use href="#id" /></svg>`
  against the `<symbol>` sets injected from `../res/img/generated/*.svg` by
  `vite.config.js` (`injectSprites`). NEVER create an `icons.jsx` / `*-icons.jsx` /
  `Icon*.jsx` module or inline `<path>` glyph markup in a `.jsx` file. Full rule:
  `.claude/rules/icons.md`.

## Native bridge

Settings protocol key table: `plans/003-settings-native-protocol-parity.md`.
Native handlers:

- `win-linux/src/cascapplicationmanagerwrapper.cpp` — `applySettings` (settings:apply handler)
- `win-linux/src/prop/cmainpanelimpl.cpp` — settings:init payload shape

l10n tables are injected via `<script>` tags in `index.html`, filling `window.l10n`.

## Comment policy

No inline comments unless logic is non-obvious. Meaningful names over comments.
