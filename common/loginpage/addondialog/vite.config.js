import { mkdirSync, readFileSync, writeFileSync } from "node:fs";
import { join } from "node:path";
import { fileURLToPath } from "node:url";
import { defineConfig } from "vite";
import solid from "vite-plugin-solid";
import { viteSingleFile } from "vite-plugin-singlefile";

const DEV_PORT = 3001;

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
    redirectRootToAddon(),
    mode === "development" && writeDevHtml(DEV_PORT),
    mode === "production" && viteSingleFile({ removeViteModuleLoader: true }),
  ].filter(Boolean),
  resolve: {
    alias: {
      "@": fileURLToPath(new URL("./src", import.meta.url)),
    },
  },
}));
