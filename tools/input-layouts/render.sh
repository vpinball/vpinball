#!/bin/sh
# Render the input device layout SVGs to the PNGs shipped in src/assets/input/
# and the .layout anchor files loaded at runtime.
# Requires Inkscape (https://inkscape.org) on PATH, or set INKSCAPE to its binary.
set -e
cd "$(dirname "$0")"
export INKSCAPE="${INKSCAPE:-inkscape}"
# PNGs (exports a copy without the 'anchors'/'callouts' metadata layers)
"${PYTHON:-python}" export.py
# 'anchors'/'callouts' layers -> src/assets/input/*.layout
"${PYTHON:-python}" layout.py
