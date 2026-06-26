import { readFileSync, writeFileSync, readdirSync, mkdirSync } from "node:fs";
import { join, basename, dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const _dirname = fileURLToPath(new URL(".", import.meta.url));
const ROOT = join(_dirname, "..");

// Mirrors grunt svg_sprite config from build/sprites/sprites.js.
// Sources and dest names must match the grunt tasks exactly.
const SPRITE_CONFIGS = [
  { pattern: "res/img/connect*.svg",         dest: "res/img/generated/connect.svg",       postProcess: fixFillWhite },
  { pattern: "res/img/welcome*.svg",         dest: "res/img/generated/welcome.svg" },
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

export function generateSpritesPlugin() {
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

// Standalone: node ./build/generate-sprites.js
if (process.argv[1] === fileURLToPath(import.meta.url)) {
  for (const config of SPRITE_CONFIGS) buildSprite(config);
  console.log(`Generated ${SPRITE_CONFIGS.length} sprites.`);
}
