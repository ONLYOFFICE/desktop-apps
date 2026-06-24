import { mkdirSync, readFileSync, readdirSync, writeFileSync, existsSync } from "node:fs";
import { join } from "node:path";
import { fileURLToPath } from "node:url";
import { defineConfig } from "vite";
import { minify } from "html-minifier-terser";
import solid from "vite-plugin-solid";
import { viteSingleFile } from "vite-plugin-singlefile";

const DEV_PORT = 3000;
const _dirname = fileURLToPath(new URL(".", import.meta.url));

function lessColors() {
  const RE_FADE = /fade\(\s*(#[0-9a-fA-F]{3,6})\s*,\s*(\d+(?:\.\d+)?)%\s*\)/g;
  const RE_DARKEN = /darken\(\s*(#[0-9a-fA-F]{3,6})\s*,\s*(\d+(?:\.\d+)?)%\s*\)/g;
  const RE_LIGHTEN = /lighten\(\s*(#[0-9a-fA-F]{3,6})\s*,\s*(\d+(?:\.\d+)?)%\s*\)/g;

  function hexToRgb(hex) {
    let h = hex.slice(1);
    if (h.length === 3) h = h[0] + h[0] + h[1] + h[1] + h[2] + h[2];
    return [parseInt(h.slice(0, 2), 16), parseInt(h.slice(2, 4), 16), parseInt(h.slice(4, 6), 16)];
  }

  function rgbToHsl(r, g, b) {
    r /= 255; g /= 255; b /= 255;
    const max = Math.max(r, g, b), min = Math.min(r, g, b);
    let h = 0, s = 0;
    const l = (max + min) / 2;
    if (max !== min) {
      const d = max - min;
      s = l > 0.5 ? d / (2 - max - min) : d / (max + min);
      switch (max) {
        case r: h = ((g - b) / d + (g < b ? 6 : 0)) / 6; break;
        case g: h = ((b - r) / d + 2) / 6; break;
        case b: h = ((r - g) / d + 4) / 6; break;
      }
    }
    return [h * 360, s * 100, l * 100];
  }

  function hslToHex(h, s, l) {
    h /= 360; s /= 100; l /= 100;
    let r, g, b;
    if (s === 0) {
      r = g = b = l;
    } else {
      const q = l < 0.5 ? l * (1 + s) : l + s - l * s;
      const p = 2 * l - q;
      const hue = (t) => {
        if (t < 0) t += 1;
        if (t > 1) t -= 1;
        if (t < 1 / 6) return p + (q - p) * 6 * t;
        if (t < 1 / 2) return q;
        if (t < 2 / 3) return p + (q - p) * (2 / 3 - t) * 6;
        return p;
      };
      r = hue(h + 1 / 3); g = hue(h); b = hue(h - 1 / 3);
    }
    return "#" + [r, g, b].map((v) => Math.round(v * 255).toString(16).padStart(2, "0")).join("");
  }

  function adjustL(hex, delta) {
    const [r, g, b] = hexToRgb(hex);
    let [h, s, l] = rgbToHsl(r, g, b);
    l = Math.min(100, Math.max(0, l + delta));
    return hslToHex(h, s, l);
  }

  function applyTransforms(code) {
    if (!code.includes("fade(") && !code.includes("darken(") && !code.includes("lighten(")) return null;
    return code
      .replace(RE_FADE, (_, hex, pct) => {
        const [r, g, b] = hexToRgb(hex);
        return `rgba(${r}, ${g}, ${b}, ${parseFloat(pct) / 100})`;
      })
      .replace(RE_DARKEN, (_, hex, pct) => adjustL(hex, -parseFloat(pct)))
      .replace(RE_LIGHTEN, (_, hex, pct) => adjustL(hex, +parseFloat(pct)));
  }

  return {
    name: "less-color-functions",
    //
    //
    enforce: "post",
    transform(code, id) {
      if (!id.split("?")[0].endsWith(".css")) return null;
      const result = applyTransforms(code);
      return result ? { code: result, map: null } : null;
    },
    //
    generateBundle(_, bundle) {
      for (const file of Object.values(bundle)) {
        if (file.type === "asset" && typeof file.source === "string" && file.fileName.endsWith(".css")) {
          const result = applyTransforms(file.source);
          if (result) file.source = result;
        }
      }
    },
  };
}

function inlineExternalHtmlResources() {
  const SCRIPT_RE = /<script(?![^>]*\btype=["']module["'])[^>]+\bsrc=["'](\.\.\/[^"']+)["'][^>]*><\/script>/g;
  const LINK_RE = /<link\b[^>]*\brel=["']stylesheet["'][^>]*\bhref=["'](\.\.\/[^"']+)["'][^>]*\/?>/g;
  let saved = [];

  return [
    {
      name: "inline-external-html-pre",
      apply: "build",
      transformIndexHtml: {
        order: "pre",
        handler(html) {
          saved = [];
          html = html.replace(SCRIPT_RE, (_, path) => { saved.push({ tag: "script", path }); return ""; });
          html = html.replace(LINK_RE, (_, path) => { saved.push({ tag: "style", path }); return ""; });
          return html;
        },
      },
    },
    {
      name: "inline-external-html-post",
      apply: "build",
      enforce: "post",
      async generateBundle(_, bundle) {
        if (!saved.length) return;
        const htmlAsset = bundle["index.html"];
        if (htmlAsset?.type !== "asset") return;

        const srcDir = fileURLToPath(new URL("../src", import.meta.url));
        const tags = saved.map(({ tag, path }) => {
          if (path.includes('noconnect/code.js')) {
            if (existsSync("../deploy/noconnect.js"))
              path = "../deploy/noconnect.js"
          } else if (path.includes('noconnect/l10n/ru.js'))
            return "";

          const absPath = join(srcDir, path);
          try {
            const content = readFileSync(absPath, "utf-8");
            return `<${tag}>${content}</${tag}>`;
          } catch {
            return tag === "script"
              ? `<script src="${path}"></script>`
              : `<link rel="stylesheet" href="${path}">`;
          }
        });

        const html = String(htmlAsset.source).replace(
          "</body>",
          `    ${tags.join("\n    ")}\n  </body>`,
        );

        htmlAsset.source = await minify(html, {
          collapseWhitespace: true,
          removeComments: true,
          removeRedundantAttributes: true,
          removeScriptTypeAttributes: true,
          removeStyleLinkTypeAttributes: true,
          minifyCSS: true,
          minifyJS: true,
        });
      },
    },
  ];
}

function injectSprites() {
  return {
    name: "inject-sprites",
    transformIndexHtml(html) {
      const spritesDir = join(_dirname, "../res/img/generated");
      let svgContent = "";
      try {
        const files = readdirSync(spritesDir).filter((f) => f.endsWith(".svg"));
        for (const file of files) {
          svgContent += readFileSync(join(spritesDir, file), "utf-8");
        }
      } catch {
        // sprites not available
      }
      if (!svgContent) return html;
      return html.replace('<div id="root"></div>', `${svgContent}\n<div id="root"></div>`);
    },
  };
}

function writeDevHtml(port) {
  return {
    name: "write-dev-html",
    configureServer(server) {
      server.httpServer?.once("listening", () => {
        const outDir = join(_dirname, "../deploy");
        const html = readFileSync(join(_dirname, "../src/index.html"), "utf-8");

        const spritesDir = join(_dirname, "../res/img/generated");
        let svgContent = "";
        try {
          const files = readdirSync(spritesDir).filter((f) => f.endsWith(".svg"));
          for (const file of files) svgContent += readFileSync(join(spritesDir, file), "utf-8");
        } catch {
          /* ok */
        }

        const withSprites = svgContent ? html.replace('<div id="root"></div>', `${svgContent}\n<div id="root"></div>`) : html;

        const devHtml = withSprites.replace(
          '<script type="module" src="./main.jsx"></script>',
          `<script type="module" src="http://localhost:${port}/@vite/client"></script>\n` +
            `<script type="module" src="http://localhost:${port}/main.jsx"></script>`,
        );

        mkdirSync(outDir, { recursive: true });
        writeFileSync(join(outDir, "index.html"), devHtml);
      });
    },
  };
}

export default defineConfig(({ mode }) => ({
  root: fileURLToPath(new URL("../src", import.meta.url)),
  base: "./",
  server: {
    port: DEV_PORT,
    cors: { origin: "*" },
  },
  plugins: [
    lessColors(),
    injectSprites(),
    inlineExternalHtmlResources(),
    solid(),
    mode === "development" && writeDevHtml(DEV_PORT),
    mode === "production" && viteSingleFile({ removeViteModuleLoader: true }),
  ].filter(Boolean),
  css: {
    transformer: "lightningcss",
  },
  build: {
    outDir: "../deploy",
    target: "es2022",
    cssMinify: "lightningcss",
    assetsInlineLimit: 100_000_000,
  },
  resolve: {
    alias: {
      "@": fileURLToPath(new URL("../src", import.meta.url)),
    },
  },
}));
