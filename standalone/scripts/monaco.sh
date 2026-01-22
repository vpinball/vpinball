#!/bin/bash

set -e

BUILD_DIR="monaco-build-temp"
OUTPUT_DIR="../../../src/assets/web/monaco"

rm -rf "$BUILD_DIR" || true
rm -rf "$OUTPUT_DIR" || true

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cat << 'EOF' > package.json
{
  "type": "module",
  "scripts": {
    "build": "vite build"
  },
  "dependencies": {
    "monaco-editor": "^0.55.1"
  },
  "devDependencies": {
    "vite": "^5.0.0"
  }
}
EOF

cat << 'EOF' > vite.config.js
import { defineConfig } from 'vite';

export default defineConfig({
  build: {
    outDir: 'dist',
    minify: 'esbuild',
    lib: {
      entry: 'main.js',
      name: 'MonacoEditor',
      fileName: 'monaco',
      formats: ['umd']
    },
    rollupOptions: {
      output: {
        globals: {
          'monaco-editor': 'monaco'
        }
      }
    }
  }
});
EOF

cat << 'EOF' > main.js
import * as monaco from 'monaco-editor/esm/vs/editor/editor.api';
import 'monaco-editor/esm/vs/basic-languages/vb/vb.contribution';
import 'monaco-editor/esm/vs/basic-languages/ini/ini.contribution';
import 'monaco-editor/esm/vs/basic-languages/xml/xml.contribution';
import 'monaco-editor/esm/vs/editor/contrib/find/browser/findController';

window.monaco = monaco;
EOF

npm install
npm run build

echo "Contents of dist directory:"
ls -la dist/

mkdir -p "$OUTPUT_DIR"
cp dist/monaco.umd.cjs "$OUTPUT_DIR/monaco.js"
cp dist/*.css "$OUTPUT_DIR/" 2>/dev/null || true

# Copy codicon font needed for find icons
CODICON_PATH="node_modules/monaco-editor/esm/vs/base/browser/ui/codicons/codicon"
if [ -f "${CODICON_PATH}/codicon.ttf" ]; then
    cp "${CODICON_PATH}/codicon.ttf" "$OUTPUT_DIR/"
fi

cd ..
rm -rf "$BUILD_DIR"

echo "Monaco UMD build complete!"
echo "Files created in $OUTPUT_DIR:"
ls -la "$OUTPUT_DIR"
