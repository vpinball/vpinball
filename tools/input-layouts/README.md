# Input device layout artwork: sources

Vector sources of the device diagrams shown on the in-game UI input device
page (`InputDevicePage`, rendered by `DeviceLayouts`). The WebP exports live in
`src/assets/input/` because everything under `src/assets` is copied into the
packaged app.

- Authored SVG, one file per recognized device in `svg/`, named like the image
  it produces (the `id` of the matching `DeviceLayout` entry in
  `src/ui/live/ingameui/DeviceLayouts.cpp`).
- Light gray line art on a transparent background, drawn to be shown on the
  dark in-game UI panels.
- Anchor coordinates are normalized (0..1) over the artwork bounds, so the
  images may be re-exported at other resolutions without side effects.

## Anchors: the 'anchors' layer

Element anchors are authored in the SVG: each file carries an Inkscape layer
(or group) labeled `anchors`, kept hidden since it is metadata, not artwork.
Every object inside it marks one input element; the center of its bounding
box is the anchor position (a small invisible rect is enough).

The object's Inkscape label (or id) declares which element it anchors:

- `"<name1>;<name2>;...|<caption>"`: one or more registered element names
  (as returned by `GetDeviceElementName`, e.g. `Left Trigger`, `Left
  Thumbstick Y`, `A`, `Space`), plus an optional short caption drawn on the
  artwork after `|`.
- `"#<id>|<caption>"`: a fixed numeric element id (decimal or `0x` hex) —
  Arcade2TV-XR pins, SDL scancodes, mouse buttons.

`layout.py` extracts the layer and emits `src/assets/input/<name>.layout`,
a JSON file the app loads at runtime; it overrides the compiled-in table in
`DeviceLayouts.cpp`, so iterating on anchors needs no rebuild. SVGs without
an `anchors` layer are skipped (the keyboard layout stays code-generated).

## Callout links: the 'callouts' layer

Where each anchor's mapping text block (the list of actions it triggers) is
drawn can also be authored: a second hidden layer/group labeled `callouts`
holds one object per anchor — a rect is enough — whose bounding box becomes
the text block position and size.

- The object's Inkscape label names the anchor it links to: any of the
  anchor's element names (`Left Trigger`) or its `"#<id>"`.
- Objects may extend past the page edges: the runtime reserves margins
  around the artwork for callouts, and normalized coordinates below 0 or
  above 1 land there.
- The leader line from the anchor to the nearest edge of the block is drawn
  at runtime (it adapts to pressed/hover states), so only the endpoints are
  authored, not the line itself.

Anchors without a matching callout object keep the automatic placement
(nearest edge, packed), so the layer can be filled in gradually.

## Export

Run `./render.sh` (requires Inkscape, or set `INKSCAPE=/path/to/inkscape`;
uses `python` for `layout.py`, or set `PYTHON`):

```sh
for svg in svg/*.svg; do
   name="$(basename "$svg" .svg)"
   inkscape "$svg" --export-type=png --export-filename="$name.png" -w 1024
   python -c "import sys; from PIL import Image; Image.open(sys.argv[1]).convert('RGBA').save(sys.argv[2], lossless=True, quality=100, method=6, exact=True)" "$name.png" "../../src/assets/input/$name.webp"
   rm "$name.png"
done
python layout.py
```

The images are rendered 1024 px wide with straight (non-premultiplied) alpha,
and stored as lossless WebP with `exact` set: libwebp otherwise drops the color
of fully transparent pixels, which bilinear filtering reads at the edges.

## Generated SVGs

`keyboard.svg` is not drawn by hand: `gen_keyboard.py` rebuilds it from a
canonical ANSI layout (keys, `anchors` layer, seeded `callouts` layer) and also
prints the `KeyCell` table compiled into `DeviceLayouts.cpp`'s
`BuildKeyboardLayout` fallback. Keep both in sync when the key set changes.

