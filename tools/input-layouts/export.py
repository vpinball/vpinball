#!/usr/bin/env python3
# Export the device layout SVGs to src/assets/input/<name>.webp (lossless, keeping the
# color of fully transparent pixels which bilinear filtering of the straight alpha reads).
#
# The 'anchors' and 'callouts' layers are authoring metadata, not artwork:
# they are stripped from a temporary copy before export, so their display
# state in the SVG does not matter (they can stay visible while editing).
#
# Requires Inkscape on PATH (or INKSCAPE=/path/to/inkscape) and Pillow.
# Usage: ./export.py

import os
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET

from PIL import Image

INKSCAPE = os.environ.get("INKSCAPE", "inkscape")
SVG_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "svg")
OUT_DIR = os.path.join(SVG_DIR, "..", "..", "..", "src", "assets", "input")
METADATA = ("anchors", "callouts")
SVGNS = "http://www.w3.org/2000/svg"
INK_LABEL = "{http://www.inkscape.org/namespaces/inkscape}label"

# Preserve readable namespace prefixes on write
ET.register_namespace("", SVGNS)
ET.register_namespace("inkscape", "http://www.inkscape.org/namespaces/inkscape")
ET.register_namespace("sodipodi", "http://sodipodi.sourceforge.net/DTD/sodipodi-0.0.0.dtd")


def is_metadata_group(e):
    return e.tag == f"{{{SVGNS}}}g" and (e.get(INK_LABEL) in METADATA or e.get("id") in METADATA)


def strip_metadata(root):
    """Remove metadata groups anywhere in the tree."""
    for e in root.iter():
        for c in [c for c in e if is_metadata_group(c)]:
            e.remove(c)


def main():
    for svg_file in sorted(os.listdir(SVG_DIR)):
        if not svg_file.endswith(".svg"):
            continue
        name = svg_file[:-4]
        root = ET.parse(os.path.join(SVG_DIR, svg_file)).getroot()
        strip_metadata(root)
        with tempfile.NamedTemporaryFile("w", suffix=".svg", delete=False, encoding="utf-8") as tmp:
            tmp.write(ET.tostring(root, encoding="unicode"))
            tmp_path = tmp.name
        tmp_png = tmp_path[:-4] + ".png"
        try:
            subprocess.run(
                [INKSCAPE, tmp_path, "--export-type=png", f"--export-filename={tmp_png}", "-w", "1024"],
                check=True,
            )
            out_webp = os.path.join(OUT_DIR, name + ".webp")
            with Image.open(tmp_png) as im:
                im.convert("RGBA").save(out_webp, lossless=True, quality=100, method=6, exact=True)
            print(f"{name}: -> {os.path.relpath(out_webp)}")
        finally:
            os.unlink(tmp_path)
            if os.path.exists(tmp_png):
                os.unlink(tmp_png)


if __name__ == "__main__":
    sys.exit(main())
