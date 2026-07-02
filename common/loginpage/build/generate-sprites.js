import { readFileSync, writeFileSync, readdirSync, mkdirSync } from "node:fs";
import { join, basename, dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import { createRequire } from "node:module";

const _require = createRequire(import.meta.url);

const _dirname = fileURLToPath(new URL(".", import.meta.url));
const ROOT = join(_dirname, "..");

// Mirrors grunt svg_sprite config from build/sprites/sprites.js.
// Sources and dest names must match the grunt tasks exactly.
const SPRITE_CONFIGS = [
  { pattern: "res/img/formats-svg/*.svg",    dest: "res/img/generated/formats.svg" },
  { pattern: "res/img/common-svg/*.svg",     dest: "res/img/generated/common.svg" },
  { pattern: "res/img/update_status/*.svg",  dest: "res/img/generated/update_status.svg" },
  { pattern: "res/img/idx-logo*.svg",        dest: "res/img/generated/logo.svg" },
  { pattern: "res/img/template-type/*.svg",  dest: "res/img/generated/templatetype.svg" },
  { pattern: "res/img/toolicons/1x/*.svg",   dest: "res/img/generated/toolicons.svg" },
  { pattern: "res/img/toolicons20/1x/*.svg", dest: "res/img/generated/toolicons20.svg" },
];

// grunt task replace_allconnect: fill="#fff" → fill="white" to avoid orange tint on light theme
function fixFillWhite(content) {
  return content.replace(/fill="#fff"/gi, 'fill="white"');
}

function makePatternRegex(filePattern) {
  return new RegExp(
    "^" +
    filePattern.replace(/[.+^${}()|[\]\\]/g, "\\$&").replace(/\*/g, "[^/]*") +
    "$"
  );
}

function resolveFiles(pattern) {
  const full = join(ROOT, pattern);
  const dir = dirname(full);
  const regex = makePatternRegex(basename(full));
  try {
    return readdirSync(dir)
      .filter(f => regex.test(f))
      .sort()
      .map(f => join(dir, f));
  } catch {
    return [];
  }
}

function svgToSymbol(content, id) {
  content = content.replace(/<\?xml[\s\S]*?\?>/gi, "").trim();

  // Match opening <svg ...> — may span multiple lines
  const svgOpenMatch = content.match(/<svg([\s\S]*?)>/i);
  if (!svgOpenMatch) return null;

  // Keep all svg attributes except xmlns declarations (those go on the root)
  const attrs = svgOpenMatch[1]
    .replace(/\s+xmlns(?::[a-z]+)?="[^"]*"/gi, "")
    .trim();

  const openEnd = content.indexOf(svgOpenMatch[0]) + svgOpenMatch[0].length;
  const closeStart = content.lastIndexOf("</svg>");
  if (closeStart === -1) return null;
  const inner = content.slice(openEnd, closeStart).trim();

  // Symbol id = filename without extension (grunt shape.id.separator: "")
  return `<symbol id="${id}"${attrs ? ` ${attrs}` : ""}>${inner}</symbol>`;
}

function buildSprite({ pattern, dest, postProcess }) {
  const files = resolveFiles(pattern);
  if (!files.length) return;

  const symbols = files
    .map(f => svgToSymbol(readFileSync(f, "utf-8"), basename(f, ".svg")))
    .filter(Boolean);

  // Root SVG matches grunt svg.rootAttributes: xmlns + fill=none
  let output =
    `<svg xmlns="http://www.w3.org/2000/svg" fill="none" width="0" height="0" style="position:absolute">\n` +
    symbols.join("\n") +
    `\n</svg>`;

  if (postProcess) output = postProcess(output);

  const destPath = join(ROOT, dest);
  mkdirSync(dirname(destPath), { recursive: true });
  writeFileSync(destPath, output, "utf-8");
}

export function generateSvgSpritesPlugin() {
  return {
    name: "generate-sprites",

    buildStart() {
      for (const config of SPRITE_CONFIGS) buildSprite(config);
    },

    configureServer(server) {
      // Watch all source directories so dev server reacts to SVG changes
      const watchDirs = [
        ...new Set(SPRITE_CONFIGS.map(c => join(ROOT, dirname(c.pattern)))),
      ];
      for (const dir of watchDirs) server.watcher.add(dir);

      const rebuild = (file) => {
        const fileBase = basename(file);
        const fileDir = resolve(dirname(file));
        let needsReload = false;

        for (const config of SPRITE_CONFIGS) {
          const patternFull = join(ROOT, config.pattern);
          if (resolve(dirname(patternFull)) !== fileDir) continue;
          if (!makePatternRegex(basename(patternFull)).test(fileBase)) continue;
          buildSprite(config);
          needsReload = true;
        }

        if (needsReload) server.ws.send({ type: "full-reload" });
      };

      server.watcher.on("change", rebuild);
      server.watcher.on("add", rebuild);
      server.watcher.on("unlink", rebuild);
    },
  };
}

// 1.25x carries per-icon position rules (shared across all scales)
const PNG_SPRITE_CONFIGS = [
  { iconsDir: "res/img/toolicons/1.25x",  spriteName: "toolicons",  scale: 1.25, wrapperClass: ".pixel-ratio__1_25", iconClasses: [".tool-icon", ".menu__icon"], positionClass: ".tool-icon" },
  { iconsDir: "res/img/toolicons/1.5x",   spriteName: "toolicons",  scale: 1.5,  wrapperClass: ".pixel-ratio__1_5",  iconClasses: [".tool-icon", ".menu__icon"], positionClass: null },
  { iconsDir: "res/img/toolicons/1.75x",  spriteName: "toolicons",  scale: 1.75, wrapperClass: ".pixel-ratio__1_75", iconClasses: [".tool-icon", ".menu__icon"], positionClass: null },
  { iconsDir: "res/img/toolicons20/1.25x", spriteName: "toolicons20", scale: 1.25, wrapperClass: ".pixel-ratio__1_25", iconClasses: [".tool-icon__20"], positionClass: ".tool-icon__20" },
  { iconsDir: "res/img/toolicons20/1.5x",  spriteName: "toolicons20", scale: 1.5,  wrapperClass: ".pixel-ratio__1_5",  iconClasses: [".tool-icon__20"], positionClass: null },
  { iconsDir: "res/img/toolicons20/1.75x", spriteName: "toolicons20", scale: 1.75, wrapperClass: ".pixel-ratio__1_75", iconClasses: [".tool-icon__20"], positionClass: null },
  { iconsDir: "res/img/formats/1.25x", spriteName: "formats", scale: 1.25, wrapperClass: ".pixel-ratio__1_25", iconClasses: [".doc-format"], positionClass: ".doc-format" },
  { iconsDir: "res/img/formats/1.5x",  spriteName: "formats", scale: 1.5,  wrapperClass: ".pixel-ratio__1_5",  iconClasses: [".doc-format"], positionClass: null },
  { iconsDir: "res/img/formats/1.75x", spriteName: "formats", scale: 1.75, wrapperClass: ".pixel-ratio__1_75", iconClasses: [".doc-format"], positionClass: null },
];

const CSS_OUT_DIR = "src/styles/sprites";

function px(v) {
  if (v === 0) return "0";
  const r = Math.round(v * 100) / 100;
  return `${r}px`;
}

function getPngFiles(dir) {
  const full = join(ROOT, dir);
  try {
    return readdirSync(full)
      .filter(f => f.endsWith(".png") && !f.startsWith("."))
      .sort()
      .map(f => join(full, f));
  } catch {
    return [];
  }
}

function composePng(files) {
  const { PNG } = _require("pngjs");
  const pngs = files.map(f => PNG.sync.read(readFileSync(f)));
  const width = Math.max(...pngs.map(p => p.width));
  const totalHeight = pngs.reduce((h, p) => h + p.height, 0);

  const out = new PNG({ width, height: totalHeight });
  out.data.fill(0);

  const yOffsets = [];
  let y = 0;
  for (const png of pngs) {
    yOffsets.push(y);
    PNG.bitblt(png, out, 0, 0, png.width, png.height, 0, y);
    y += png.height;
  }

  return { buffer: PNG.sync.write(out), width, yOffsets };
}

function buildPngSprite(config) {
  const files = getPngFiles(config.iconsDir);
  if (!files.length) return;

  const { buffer, width, yOffsets } = composePng(files);

  // PNG sprite sheet
  const pngDest = join(ROOT, "res/img/generated", `${config.spriteName}@${config.scale}x.png`);
  mkdirSync(dirname(pngDest), { recursive: true });
  writeFileSync(pngDest, buffer);

  // CSS — background-image embedded as base64 (mirrors grunt destCss with data-uri)
  const base64 = buffer.toString("base64");
  const cssWidth = width / config.scale;
  const selectors = config.iconClasses.map(c => `${config.wrapperClass} ${c}`).join(",\n");

  let css = `${selectors} {\n`;
  css += `    background-image: url('data:image/png;base64,${base64}');\n`;
  css += `    background-size: ${px(cssWidth)} auto;\n`;
  css += `}\n`;

  // Per-icon background-position-y — only in 1.25x; positions are in CSS px and
  // remain correct for 1.5x/1.75x because icon CSS size is fixed across scales.
  if (config.positionClass) {
    for (let i = 0; i < files.length; i++) {
      const name = basename(files[i], ".png");
      const offsetCss = yOffsets[i] === 0 ? "0" : px(-(yOffsets[i] / config.scale));
      css += `${config.positionClass}.${name} { background-position-y: ${offsetCss}; }\n`;
    }
  }

  const cssDest = join(ROOT, CSS_OUT_DIR, `${config.spriteName}@${config.scale}x.css`);
  mkdirSync(dirname(cssDest), { recursive: true });
  writeFileSync(cssDest, css, "utf-8");
}

export function generatePngSpritesPlugin() {
  return {
    name: "generate-png-sprites",

    buildStart() {
      for (const config of PNG_SPRITE_CONFIGS) buildPngSprite(config);
    },

    configureServer(server) {
      const watchDirs = [...new Set(PNG_SPRITE_CONFIGS.map(c => join(ROOT, c.iconsDir)))];
      for (const dir of watchDirs) server.watcher.add(dir);

      const rebuild = (file) => {
        if (!file.endsWith(".png")) return;
        const fileDir = resolve(dirname(file));
        let needsReload = false;
        for (const config of PNG_SPRITE_CONFIGS) {
          if (resolve(join(ROOT, config.iconsDir)) === fileDir) {
            buildPngSprite(config);
            needsReload = true;
          }
        }
        if (needsReload) server.ws.send({ type: "full-reload" });
      };

      server.watcher.on("change", rebuild);
      server.watcher.on("add", rebuild);
      server.watcher.on("unlink", rebuild);
    },
  };
}

// Standalone: node ./build/generate-sprites.js
if (process.argv[1] === fileURLToPath(import.meta.url)) {
  for (const config of SPRITE_CONFIGS) buildSprite(config);
  for (const config of PNG_SPRITE_CONFIGS) buildPngSprite(config);
  console.log(`Generated ${SPRITE_CONFIGS.length} SVG + ${PNG_SPRITE_CONFIGS.length} PNG sprites.`);
}
