#!/usr/bin/env python3
# Regenerates tools/input-layouts/svg/keyboard.svg from a canonical ANSI layout:
# keys group (artwork), 'anchors' layer (#scancode|caption per key center) and a
# 'callouts' layer seeded in the margins. Also prints the C++ KeyCell table used
# by the compiled fallback in DeviceLayouts.cpp (BuildKeyboardLayout).
import re

scancodes = {}
for m in re.finditer(r"SDL_SCANCODE_(\w+)\s*=\s*(\d+)", open("third-party/include/SDL3/SDL_scancode.h", encoding="utf-8").read()):
    scancodes[m.group(1)] = int(m.group(2))

# (scancode, caption, x, y, w, h) in key units (1u = 40px)
K = []

def row(keys, y):
    x = 0.0
    for scan, cap, w in keys:
        K.append((scan, cap, x, y, w, 1.0))
        x += w

# F row (y=0): Esc, gap, F1-4, gap, F5-8, gap, F9-12, gap, PrtSc ScrLk Pause
K.append(("ESCAPE", "Esc", 0, 0, 1, 1))
for i, s in enumerate(["F1", "F2", "F3", "F4"]):
    K.append((s, s, 2 + i, 0, 1, 1))
for i, s in enumerate(["F5", "F6", "F7", "F8"]):
    K.append((s, s, 6.5 + i, 0, 1, 1))
for i, s in enumerate(["F9", "F10", "F11", "F12"]):
    K.append((s, s, 11 + i, 0, 1, 1))
for i, (s, c) in enumerate([("PRINTSCREEN", "PrtSc"), ("SCROLLLOCK", "ScrLk"), ("PAUSE", "Pause")]):
    K.append((s, c, 15.5 + i, 0, 1, 1))

# Main block (y=1.5..5.5), ANSI widths, 15u wide
row([("GRAVE", "`", 1)] + [(str(i), str(i), 1) for i in range(1, 10)]
    + [("0", "0", 1), ("MINUS", "-", 1), ("EQUALS", "=", 1), ("BACKSPACE", "Bksp", 2)], 1.5)
row([("TAB", "Tab", 1.5)] + [(c, c, 1) for c in "QWERTYUIOP"]
    + [("LEFTBRACKET", "[", 1), ("RIGHTBRACKET", "]", 1), ("BACKSLASH", "\\", 1.5)], 2.5)
row([("CAPSLOCK", "Caps", 1.75)] + [(c, c, 1) for c in "ASDFGHJKL"]
    + [("SEMICOLON", ";", 1), ("APOSTROPHE", "'", 1), ("RETURN", "Enter", 2.25)], 3.5)
row([("LSHIFT", "Shift", 2.25)] + [(c, c, 1) for c in "ZXCVBNM"]
    + [("COMMA", ",", 1), ("PERIOD", ".", 1), ("SLASH", "/", 1), ("RSHIFT", "Shift", 2.75)], 4.5)
row([("LCTRL", "Ctrl", 1.25), ("LGUI", "Win", 1.25), ("LALT", "Alt", 1.25), ("SPACE", "Space", 6.25),
     ("RALT", "Alt", 1.25), ("RGUI", "Win", 1.25), ("MENU", "Menu", 1.25), ("RCTRL", "Ctrl", 1.25)], 5.5)

# Nav cluster above arrows (x=15.5, 3u) + arrows (rows 4-5)
for i, (s, c) in enumerate([("INSERT", "Ins"), ("HOME", "Home"), ("PAGEUP", "PgUp")]):
    K.append((s, c, 15.5 + i, 1.5, 1, 1))
for i, (s, c) in enumerate([("DELETE", "Del"), ("END", "End"), ("PAGEDOWN", "PgDn")]):
    K.append((s, c, 15.5 + i, 2.5, 1, 1))
K.append(("UP", "\u2191", 16.5, 4.5, 1, 1))
for i, (s, c) in enumerate([("LEFT", "\u2190"), ("DOWN", "\u2193"), ("RIGHT", "\u2192")]):
    K.append((s, c, 15.5 + i, 5.5, 1, 1))

# Numpad (x=19, 4u): NumLk / * - | 7 8 9 +(2h) | 4 5 6 | 1 2 3 Ent(2h) | 0(2w) .
for i, (s, c) in enumerate([("NUMLOCKCLEAR", "Num"), ("KP_DIVIDE", "/"), ("KP_MULTIPLY", "*"), ("KP_MINUS", "-")]):
    K.append((s, c, 19 + i, 1.5, 1, 1))
for i in range(3):
    K.append((f"KP_{7 + i}", str(7 + i), 19 + i, 2.5, 1, 1))
K.append(("KP_PLUS", "+", 22, 2.5, 1, 2))
for i in range(3):
    K.append((f"KP_{4 + i}", str(4 + i), 19 + i, 3.5, 1, 1))
for i in range(3):
    K.append((f"KP_{1 + i}", str(1 + i), 19 + i, 4.5, 1, 1))
K.append(("KP_ENTER", "Enter", 22, 4.5, 1, 2))
K.append(("KP_0", "0", 19, 5.5, 2, 1))
K.append(("KP_PERIOD", ".", 21, 5.5, 1, 1))

U, M = 40.0, 4.0
DOCW, DOCH = 23.2 * U, 6.7 * U  # 928 x 268

svg = ['<?xml version="1.0" encoding="UTF-8" standalone="no"?>',
       f'<svg viewBox="0 0 {DOCW:.0f} {DOCH:.0f}" width="{DOCW:.0f}" height="{DOCH:.0f}" version="1.1" id="svg87"',
       '   sodipodi:docname="keyboard.svg"',
       '   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"',
       '   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"',
       '   xmlns="http://www.w3.org/2000/svg">',
       '  <sodipodi:namedview id="namedview87" pagecolor="#00009b" bordercolor="#000000" borderopacity="0.25" inkscape:showpageshadow="2" inkscape:pageopacity="0.0" inkscape:deskcolor="#d1d1d1" />',
       '  <g id="keys" fill="#141414" stroke="#c8c8c8" stroke-width="2.5" stroke-linejoin="round">']
for i, (scan, cap, x, y, w, h) in enumerate(K):
    svg.append(f'    <rect id="key{i}" x="{M + x * U:.1f}" y="{M + y * U:.1f}" width="{w * U - 4:.1f}" height="{h * U - 4:.1f}" rx="6" />')
svg.append('  </g>')
svg.append('  <g id="anchors" inkscape:groupmode="layer" inkscape:label="anchors" style="display:none">')
for i, (scan, cap, x, y, w, h) in enumerate(K):
    cx, cy = M + (x + w / 2) * U, M + (y + h / 2) * U
    svg.append(f'    <rect id="anchor{i}" inkscape:label="#{scancodes[scan]}|{cap}" x="{cx - 3:.1f}" y="{cy - 3:.1f}" width="6" height="6" style="fill:#ff0000;fill-opacity:0.35;stroke:#ff0000;stroke-width:1" />')
svg.append('  </g>')
# callouts: one starter rect per anchor in the margin nearest to it, packed
sides = [[], [], [], []]
for i, (scan, cap, x, y, w, h) in enumerate(K):
    cx, cy = M + (x + w / 2) * U, M + (y + h / 2) * U
    d = [cx, DOCW - cx, cy, DOCH - cy]
    sides[d.index(min(d))].append((cy if d.index(min(d)) < 2 else cx, i))
for side, items in enumerate(sides):
    items.sort()
    if side < 2:
        # side margins: 3 sub-columns, callouts packed vertically without overlap
        cols, CW, CH, GAP = 3, 110, 16, 2
        ys = [0.0] * cols
        for j, (pos, i) in enumerate(items):
            c = j % cols
            y = max(ys[c], pos - CH / 2)
            xs = (-CW - 10 - c * (CW + 8)) if side == 0 else (DOCW + 10 + c * (CW + 8))
            ys[c] = y + CH + GAP
            K[i] = (*K[i], (xs, y, CW, CH))
    else:
        # top/bottom bands: 4 rows, packed horizontally without overlap
        rows, CW, CH, GAP = 4, 70, 16, 8
        xs = [-0.45 * DOCW] * rows
        for j, (pos, i) in enumerate(items):
            r = j % rows
            x = max(xs[r], pos - CW / 2)
            y = (-26 - r * (CH + 4)) if side == 2 else (DOCH + 10 + r * (CH + 4))
            xs[r] = x + CW + GAP
            K[i] = (*K[i], (x, y, CW, CH))
svg.append('  <g id="callouts" inkscape:groupmode="layer" inkscape:label="callouts" style="display:none">')
for i, k in enumerate(K):
    scan, cap, x, y, w, h, rect = k
    rx, ry, rw, rh = rect
    svg.append(f'    <rect id="callout{i}" inkscape:label="#{scancodes[scan]}" x="{rx:.1f}" y="{ry:.1f}" width="{rw}" height="{rh}" style="fill:#ff0000;fill-opacity:0.25;stroke:#ff0000;stroke-width:1" />')
svg.append('  </g>')
svg.append('</svg>')
open('tools/input-layouts/svg/keyboard.svg', 'w', encoding='utf-8', newline='\n').write('\n'.join(svg))
print(f'{len(K)} keys written to keyboard.svg')

# C++ KeyCell table for the compiled fallback
def cap_cpp(c):
    if c == '\\':
        return '"\\\\"'
    if c in '\u2190\u2191\u2192\u2193':
        return '"\\xE2\\x86\\x%02X"' % (0x90 + '\u2190\u2191\u2192\u2193'.index(c))
    return '"' + c + '"'

lines = ['      static constexpr KeyCell cells[] = {']
for k in K:
    scan, cap, x, y, w, h = k[:6]
    f = lambda v: f'{float(v)}f'  # float() keeps the decimal point: 0.0f, 2.25f
    lines.append(f'         {{ SDL_SCANCODE_{scan}, {f(x)}, {f(y)}, {f(w)}, {f(h)}, {cap_cpp(cap)} }},')
lines.append('      };')
open('tools/input-layouts/cells_table.txt', 'w', encoding='utf-8', newline='\n').write('\n'.join(lines))
print('cells table written to cells_table.txt')
