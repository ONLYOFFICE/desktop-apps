import { existsSync, mkdirSync, readFileSync, readdirSync, writeFileSync } from 'node:fs'
import { dirname, join, resolve } from 'node:path'
import { fileURLToPath } from 'node:url'
import { minify as minifyHtml } from 'html-minifier-terser'
import { minify as minifyJs } from 'terser'
import { defineConfig } from 'vite'

const __dirname = dirname(fileURLToPath(import.meta.url))

function noconnectPlugin() {
  return {
    name: 'noconnect',
    apply: 'build',
    resolveId(id) {
      if (id === 'noconnect:entry') return '\0noconnect:entry'
    },
    load(id) {
      if (id === '\0noconnect:entry') return 'export default 0'
    },
    generateBundle(_, bundle) {
      for (const key in bundle) delete bundle[key]
    },
    async closeBundle() {
      const deployDir = resolve(__dirname, '../deploy')

      const l10nFiles = readdirSync(join(__dirname, 'l10n'))
        .filter(f => f.endsWith('.js'))
        .sort()
        .map(f => readFileSync(join(__dirname, 'l10n', f), 'utf-8'))

      const jsCode = [
        readFileSync(join(__dirname, 'code.js'), 'utf-8'),
        ...l10nFiles,
        readFileSync(join(__dirname, 'init.js'), 'utf-8'),
      ].join('\n')

      const { code: minJs } = await minifyJs(jsCode, {
        compress: true,
        mangle: true,
      })
      mkdirSync(deployDir, { recursive: true })
      writeFileSync(join(deployDir, 'noconnect.js'), minJs)

      let html = readFileSync(join(__dirname, 'index.html.deploy'), 'utf-8')

      html = html.replace(
        /<img\s+class="inline-svg"\s+src="([^"]+)"\s*\/?>/gi,
        (_, imgSrc) => {
          const svgPath = join(__dirname, imgSrc)
          return existsSync(svgPath) ? readFileSync(svgPath, 'utf-8').trim() : ''
        },
      )

      html = html.replace(
        /<script\b[^>]*\breplace\b[^>]*><\/script>/gi,
        `<script>${minJs}</script>`,
      )

      const result = await minifyHtml(html, {
        collapseWhitespace: true,
        decodeEntities: false,
        minifyCSS: true,
        minifyJS: false,
        removeComments: true,
      })

      writeFileSync(join(deployDir, 'noconnect.html'), result)
    },
  }
}

export default defineConfig({
  build: {
    emptyOutDir: false,
    outDir: resolve(__dirname, '../deploy'),
    rollupOptions: {
      input: 'noconnect:entry',
      output: {
        dir: resolve(__dirname, '../deploy'),
      },
    },
  },
  plugins: [noconnectPlugin()],
})
