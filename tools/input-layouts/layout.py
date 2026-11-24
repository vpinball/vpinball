#!/usr/bin/env python3
# Extract device layout anchors from the 'anchors' layer of each SVG and emit
# src/assets/input/<name>.layout files loaded at runtime by DeviceLayouts.
#
# Authoring convention (inside each svg/*.svg):
#   - An Inkscape layer or group labeled 'anchors' (inkscape:label or id
#     "anchors"). Keep it hidden: it is metadata, not artwork.
#   - Each child object inside it marks one input element; its bounding box
#     center is the anchor position. A small invisible rect is enough.
#   - The child's inkscape:label (fallback: its id) is:
#       "<name1>;<name2>;...|<caption>"   element name(s), optional caption
#       "#<id>|<caption>"                 fixed numeric element id (dec or 0x hex)
#     Element names are the ones registered in VPX (see GetDeviceElementName):
#     e.g. "Left Trigger", "Left Thumbstick Y", "A", "Space".
#   - An optional second layer/group labeled 'callouts' authors where each
#     mapping text block is drawn: one child object (a rect is enough) per
#     anchor, labeled with any of the anchor's element names or its "#<id>".
#     Its bounding box becomes the text block; objects may extend off-canvas
#     into the margins the runtime adds around the artwork. The leader line
#     (anchor -> block) is drawn at runtime. Anchors without a callout object
#     are placed automatically around the closest artwork edge.
#
# Requires Inkscape on PATH (or INKSCAPE=/path/to/inkscape) for bounding boxes.
# Usage: ./layout.py

import json
import os
import re
import subprocess
import sys
import xml.etree.ElementTree as ET

INKSCAPE = os.environ.get("INKSCAPE", "inkscape")
SVG_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "svg")
OUT_DIR = os.path.join(SVG_DIR, "..", "..", "..", "src", "assets", "input")
INK_LABEL = "{http://www.inkscape.org/namespaces/inkscape}label"
INK_GROUP = "{http://www.inkscape.org/namespaces/inkscape}groupmode"


def svg_tag(e):
    return e.tag.split("}", 1)[1]


def query_bboxes(svg_path):
    """inkscape --query-all -> {id: (x, y, w, h)} in document (viewBox) units."""
    out = subprocess.run(
        [INKSCAPE, svg_path, "--query-all"], capture_output=True, text=True, check=True
    ).stdout
    bboxes = {}
    for line in out.splitlines():
        parts = line.rsplit(",", 4)
        if len(parts) == 5:
            try:
                bboxes[parts[0]] = tuple(float(v) for v in parts[1:])
            except ValueError:
                pass
    return bboxes


def to_px(length):
    """SVG length with unit -> px (96dpi, matching inkscape --query-all output)."""
    units = {"mm": 96 / 25.4, "cm": 96 / 2.54, "in": 96.0, "pt": 96 / 72.0, "pc": 16.0}
    match = re.fullmatch(r"([0-9.eE+-]+)\s*([a-zA-Z]*)", length.strip())
    if not match:
        return None
    return float(match.group(1)) * units.get(match.group(2), 1.0)


def document_box(root):
    """Normalized anchor space = the exported image = document extent, in the px
    units reported by `inkscape --query-all`. Returns (originX, originY, span)."""
    view_box = [0.0, 0.0, 1.0, 1.0]
    if root.get("viewBox"):
        view_box = [float(x) for x in re.split(r"[ ,]+", root.get("viewBox").strip())]
    # --query-all reports user-unit px: scale viewBox units by the px size of the page
    scale_x = scale_y = 1.0
    width_px, height_px = to_px(root.get("width", "")), to_px(root.get("height", ""))
    if width_px and view_box[2]:
        scale_x = width_px / view_box[2]
    if height_px and view_box[3]:
        scale_y = height_px / view_box[3]
    return view_box[0] * scale_x, view_box[1] * scale_y, view_box[2] * scale_x, view_box[3] * scale_y


def find_group(root, name):
    """First group/layer whose inkscape:label or id is the given name."""
    for e in root.iter():
        if svg_tag(e) == "g" and (e.get(INK_LABEL) == name or e.get("id") == name):
            return e
    return None


def labeled_elements(group):
    """All descendants carrying a name: inkscape:label first, else id."""
    found = []

    def walk(e):
        name = e.get(INK_LABEL) or e.get("id")
        if name:
            found.append((e.get("id"), name))
        for c in e:
            walk(c)

    for child in group:
        walk(child)
    return found


def parse_anchor_label(label):
    """'<names>|<caption>' -> (elementId|None, [names], caption|None)."""
    names_part, _, caption = label.partition("|")
    caption = caption.strip() or None
    element_names = [n.strip() for n in names_part.split(";") if n.strip()]
    element_id = None
    if element_names and element_names[0].startswith("#"):
        element_id = int(element_names.pop(0)[1:], 0)
    return element_id, element_names, caption


def main():
    for svg_file in sorted(os.listdir(SVG_DIR)):
        if not svg_file.endswith(".svg"):
            continue
        name = svg_file[:-4]
        svg_path = os.path.join(SVG_DIR, svg_file)
        root = ET.parse(svg_path).getroot()
        group = find_group(root, "anchors")
        if group is None:
            print(f"{name}: no 'anchors' layer, skipped")
            continue

        bboxes = query_bboxes(svg_path)
        bx, by, bw, bh = document_box(root)

        # 'callouts' layer: object bbox -> text block rect, labeled by anchor name
        callouts = []  # [(elementId|None, [names], (x, y, w, h))]
        callouts_group = find_group(root, "callouts")
        if callouts_group is not None:
            for el_id, label in labeled_elements(callouts_group):
                if el_id not in bboxes:
                    print(f"{name}: callout '{label}' has no geometry, skipped")
                    continue
                cid, cnames, _ = parse_anchor_label(label)
                callouts.append((cid, cnames, bboxes[el_id], label))

        def callout_matches(cid, cnames, element_id, element_names):
            return (cid is not None and cid == element_id) or (cnames and element_names and set(cnames) & set(element_names))

        matched_callouts = set()
        anchors = []
        for el_id, label in labeled_elements(group):
            if el_id not in bboxes:
                print(f"{name}: '{label}' has no geometry, skipped")
                continue
            x, y, w, h = bboxes[el_id]
            element_id, element_names, caption = parse_anchor_label(label)
            px, py = (x + w / 2 - bx) / bw, (y + h / 2 - by) / bh
            if not (0 <= px <= 1 and 0 <= py <= 1):
                print(f"{name}: '{label}' anchor is off-canvas ({px:.3f}, {py:.3f}), clamped")
                px, py = min(max(px, 0.0), 1.0), min(max(py, 0.0), 1.0)
            anchor = {"pos": [round(px, 4), round(py, 4)]}
            if element_id is not None:
                anchor["id"] = element_id
            if element_names:
                anchor["names"] = element_names
            if caption:
                anchor["caption"] = caption
            for cid, cnames, (cx, cy, cw, ch), clabel in callouts:
                if callout_matches(cid, cnames, element_id, element_names):
                    rect = [(cx - bx) / bw, (cy - by) / bh, cw / bw, ch / bh]
                    if not (-0.5 <= rect[0] <= 1.5 and -0.5 <= rect[1] <= 1.5):
                        print(f"{name}: callout '{clabel}' is far off-canvas, skipped")
                        matched_callouts.add(clabel)
                        break
                    anchor["callout"] = [round(v, 4) for v in rect]
                    matched_callouts.add(clabel)
                    break
            anchors.append(anchor)

        for _cid, _cn, _b, clabel in callouts:
            if clabel not in matched_callouts:
                print(f"{name}: callout '{clabel}' matches no anchor, skipped")

        if not anchors:
            print(f"{name}: 'anchors' layer is empty, skipped")
            continue
        out_path = os.path.join(OUT_DIR, name + ".layout")
        with open(out_path, "w", encoding="utf-8") as f:
            json.dump({"aspect": round(bw / bh, 4), "anchors": anchors}, f, indent=1, ensure_ascii=False)
        print(f"{name}: {len(anchors)} anchors -> {os.path.relpath(out_path)}")


if __name__ == "__main__":
    sys.exit(main())
