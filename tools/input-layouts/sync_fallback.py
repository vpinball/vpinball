#!/usr/bin/env python3
"""Re-sync the compiled-in fallback layouts in DeviceLayouts.cpp with the
generated src/assets/input/*.layout sidecars (which override it at runtime).
Anchor positions/captions/aspects are updated; element ids/names come from
the .layout files. Usage: python sync_fallback.py"""
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CPP = ROOT / 'src/ui/live/ingameui/DeviceLayouts.cpp'
ASSETS = ROOT / 'src/assets/input'

# PUA icon codepoint -> ICON_LC_* constant, parsed from IconsLucide.h
icons = {}
for m in re.finditer(r'#define (ICON_LC_\w+) ".*" // U\+([0-9A-Fa-f]{4,6})',
                     (ROOT / 'src/ui/live/IconsLucide.h').read_text(encoding='utf-8')):
    icons[chr(int(m.group(2), 16))] = m.group(1)

SDL_BUTTON = {1: 'SDL_BUTTON_LEFT', 2: 'SDL_BUTTON_RIGHT', 3: 'SDL_BUTTON_MIDDLE',
              4: 'SDL_BUTTON_X1', 5: 'SDL_BUTTON_X2'}


def cpp_string(s):
    return '"' + s.replace('\\', '\\\\').replace('"', '\\"') + '"'


def emit_anchor(a, id_style):
    pos = '{ %.4ff, %.4ff }' % (a['pos'][0], a['pos'][1])
    cap = ', %s' % cpp_string(a['caption']) if a.get('caption') else ''
    if id_style == 'const':
        eid = SDL_BUTTON.get(a['id'], '0x%04X' % a['id'])
    elif a.get('id') is not None:
        eid = '0x%04X' % a['id']
    else:
        eid = '0xFFFF'
    names = a.get('names') or []
    parts = [icons.get(ch) or cpp_string(ch) + 's' for ch in names]
    names_cpp = '{ %s }' % ', '.join(parts) if parts else '{}'
    return '      { %s, %s, %s%s },' % (eid, names_cpp, pos, cap)


def image_aspect(layout_name):
    from PIL import Image
    im = Image.open(ASSETS / (layout_name + '.webp'))
    return round(1024 * im.size[1] / im.size[0])


def sync(text, const_name, layout_name, id_style):
    d = json.loads((ASSETS / (layout_name + '.layout')).read_text(encoding='utf-8'))
    block = '\n'.join(emit_anchor(a, id_style) for a in d['anchors'])
    pat = re.compile(r'(const DeviceLayout ' + const_name + r' \{.*?\.anchors = \{\n).*?(\n   \},\n\};)', re.S)
    text, n = pat.subn(lambda m: m.group(1) + block + m.group(2), text, count=1)
    assert n == 1, const_name + ': anchors block not found'
    aspect = image_aspect(layout_name)
    pat = re.compile(r'(const DeviceLayout ' + const_name + r' \{.*?\.aspect = 1024\.f / )[\d.]+f(,)', re.S)
    text, n = pat.subn(r'\g<1>%d.f\g<2>' % aspect, text, count=1)
    assert n == 1, const_name + ': aspect not found'
    return text


def check_keyboard(text):
    d = json.loads((ASSETS / 'keyboard.layout').read_text(encoding='utf-8'))
    lay = {a['id']: (round(a['pos'][0], 4), round(a['pos'][1], 4), a.get('caption', ''))
           for a in d['anchors']}
    cells = re.findall(r'\{ (\d+), \{ ([\d.]+)f, ([\d.]+)f \}, "(.*?)" \}', text)
    bad = [(int(i), lay.get(int(i))) for i, x, y, c in cells
           if lay.get(int(i)) != (round(float(x), 4), round(float(y), 4), c.replace('\\\\', '\\'))]
    if bad:
        print('keyboard: %d/%d cells differ from .layout!' % (len(bad), len(cells)))
        for i, v in bad[:10]:
            print('   scancode', i, '->', v)
        return False
    print('keyboard: all %d cells match' % len(cells))
    return True


def main():
    text = CPP.read_text(encoding='utf-8')
    ok = check_keyboard(text)
    text = sync(text, 'kGamepadLayout', 'gamepad', 'hex')
    text = sync(text, 'kArcade2TVLayout', 'arcade2tv-xr', 'hex')
    text = sync(text, 'kVRLayout', 'vr-controller', 'hex')
    text = sync(text, 'kMouseLayout', 'mouse', 'const')
    CPP.write_text(text, encoding='utf-8')
    print('DeviceLayouts.cpp synced')
    sys.exit(0 if ok else 1)


main()
