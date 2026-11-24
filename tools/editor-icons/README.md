# Live editor part icons: sources

Vector sources of the part icons in `src/assets/editor/*.webp` (the add-part menu, outliner rows, property header and sidebar of the live editor). They live here and not in `src/assets` because everything under `src/assets` is copied into the packaged app.

- Authored SVG, one file per part in `svg/`, named like the image it produces.
- 24x24 grid (`viewBox="0 0 24 24"`), 2 px stroke, round caps and joins, `fill="none"`, `stroke="currentColor"`: the Lucide conventions, like the editor's UI glyphs (`src/ui/live/LucideFont.h`).
- Secondary detail is a 1.5 px stroke at 50% opacity; solid dots mark pivots, posts and nodes.

## Export

The editor draws the textures untinted (white `ImDrawList::AddImage` / `ImGui::Image`) on dark panels, so the images are 60x60 pure white with the shape in the alpha channel. RGB is set to white on every pixel, including the fully transparent ones, so that mipmapping and bilinear filtering of the unpremultiplied texture do not darken the edges. They are stored as lossless WebP with `exact` set, as libwebp otherwise drops the color of fully transparent pixels.

```python
# pip install cairosvg pillow
import cairosvg, io
from PIL import Image
for name in ["ball", "bumper", ...]:  # one per svg/<name>.svg
    svg = open(f"svg/{name}.svg").read().replace("currentColor", "#ffffff")
    png = cairosvg.svg2png(bytestring=svg.encode(), output_width=60, output_height=60)
    alpha = Image.open(io.BytesIO(png)).convert("RGBA").getchannel("A")
    out = Image.new("RGBA", (60, 60), (255, 255, 255, 255))
    out.putalpha(alpha)
    out.save(f"../../src/assets/editor/{name}.webp", lossless=True, quality=100, method=6, exact=True)
```
