import { mkdirSync, readFileSync, writeFileSync } from "node:fs";
import { join } from "node:path";
import { fileURLToPath } from "node:url";
import { defineConfig } from "vite";
import solid from "vite-plugin-solid";
import { viteSingleFile } from "vite-plugin-singlefile";

const DEV_PORT = 3001;

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
    enforce: "post",
    transform(code, id) {
      if (!id.split("?")[0].endsWith(".css")) return null;
      const result = applyTransforms(code);
      return result ? { code: result, map: null } : null;
    },
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

function redirectRootToAddon() {
  return {
    name: "redirect-root-to-addon",
    configureServer(server) {
      server.middlewares.use((req, res, next) => {
        if (req.url === "/") {
          res.writeHead(302, { Location: "/addon.html" });
          res.end();
          return;
        }
        next();
      });
    },
  };
}

function writeDevHtml(port) {
  return {
    name: "write-dev-html",
    configureServer(server) {
      server.httpServer?.once("listening", () => {
        const outDir = fileURLToPath(new URL("../deploy", import.meta.url));
        const html = readFileSync(fileURLToPath(new URL("./src/addon.html", import.meta.url)), "utf-8");

        const devHtml = html.replace(
          '<script type="module" src="./main.jsx"></script>',
          `<script type="module" src="http://localhost:${port}/@vite/client"></script>\n` +
            `<script type="module" src="http://localhost:${port}/main.jsx"></script>`,
        );

        mkdirSync(outDir, { recursive: true });
        writeFileSync(join(outDir, "addon.html"), devHtml);
      });
    },
  };
}

export default defineConfig(({ mode }) => ({
  root: fileURLToPath(new URL("./src", import.meta.url)),
  base: "./",
  server: {
    port: DEV_PORT,
    cors: { origin: "*" },
  },
  build: {
    outDir: fileURLToPath(new URL("../deploy", import.meta.url)),
    emptyOutDir: false,
    target: "es2022",
    assetsInlineLimit: 100_000_000,
    rollupOptions: {
      input: fileURLToPath(new URL("./src/addon.html", import.meta.url)),
    },
  },
  plugins: [
    solid(),
    lessColors(),
    redirectRootToAddon(),
    mode === "development" && writeDevHtml(DEV_PORT),
    mode === "production" && viteSingleFile({ removeViteModuleLoader: true }),
  ].filter(Boolean),
  css: {
    transformer: "lightningcss",
  },
  resolve: {
    alias: {
      "@": fileURLToPath(new URL("./src", import.meta.url)),
    },
  },
}));
