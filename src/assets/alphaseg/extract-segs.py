# This little python script generates the signed distance field atlases used for rendering segment displays
# It requires https://github.com/Chlumsky/msdfgen to be in the path

# pip install Pillow
# pip install lxml

# Alongside each SDF atlas '<name>.png', a pre-integrated coverage atlas '<name>-cov.png' is generated
# (twice as wide: left half = sharp segment coverage, right half = squared SDF) which is used by VPX for
# resolution independent minification (see GetSegCov in Renderer.cpp and fs_display.sc). To regenerate only
# the coverage atlases from the existing SDFs (no msdfgen needed), run with '--cov-only'.

from PIL import Image
import subprocess
import os
import sys
import glob

img_w = 24
img_h = 32
scale = 4
render = False

def extract_single_path(svg_file, group_id, path_index, out_img, crop_right, crop_bottom):
    from lxml import etree
    parser = etree.XMLParser(remove_blank_text=False, resolve_entities=False)
    tree = etree.parse(svg_file, parser)
    root = tree.getroot()
    namespace = { 'svg': 'http://www.w3.org/2000/svg', 'inkscape' : 'http://www.inkscape.org/namespaces/inkscape' }
    path_found = False
    for group in root.findall('svg:g', namespaces=namespace):
        if group.attrib.get('id') != 'Segments':
            root.remove(group)
        else:
            for subgroup in group.findall('svg:g', namespaces=namespace):
                if subgroup.attrib.get('{http://www.inkscape.org/namespaces/inkscape}label') != group_id:
                    group.remove(subgroup)
                else:
                    for idx, path in enumerate(reversed(subgroup)):
                        if idx != path_index:
                            subgroup.remove(path)
                        else:
                            path_found = True
    if path_found:
        tmp_file = 'tmp.svg'
        tree.write(tmp_file, pretty_print=True, encoding='utf-8', xml_declaration=True)
        if render:
            subprocess.run(["msdfgen.exe", "sdf", "-svg" , tmp_file, "-o", out_img, "-pxrange", f"{4.0 * 2.0 * scale}", "-scale", f"{scale}", "-size", f"{int(scale * (img_w - crop_right))}", f"{int(scale * (img_h - crop_bottom))}", "-testrender", f"render-{out_img}", f"{int(4 * scale * (img_w - crop_right))}", f"{int(4 * scale * (img_h - crop_bottom))}"])
        else:
            subprocess.run(["msdfgen.exe", "sdf", "-svg" , tmp_file, "-o", out_img, "-pxrange", f"{4.0 * 2.0 * scale}", "-scale", f"{scale}", "-size", f"{int(scale * (img_w - crop_right))}", f"{int(scale * (img_h - crop_bottom))}"])
        os.remove(tmp_file) 
    print(f"{group_id}-Seg{path_index}: {path_found}")
    return path_found


def build_sdf(svg_file, group_id, crop_right, crop_bottom, segs, out_file):
    crop_right = 0 #Cropping is deactivated for the time being (needs quite some change on VPX side to support adjusted SDF size)
    crop_bottom = 0
    mergedR = Image.new('L', (int(4 * scale * (img_w - crop_right)), int(scale * (img_h - crop_bottom))))
    mergedG = Image.new('L', (int(4 * scale * (img_w - crop_right)), int(scale * (img_h - crop_bottom))))
    mergedB = Image.new('L', (int(4 * scale * (img_w - crop_right)), int(scale * (img_h - crop_bottom))))
    mergedA = Image.new('L', (int(4 * scale * (img_w - crop_right)), int(scale * (img_h - crop_bottom))))
    for i, idx in enumerate(segs):
        if idx < 0:
            continue
        out_img = f"{group_id}-Seg{i}.png"
        if idx >= 0 and extract_single_path(svg_file, group_id, idx, out_img, crop_right, crop_bottom):
            image = Image.open(out_img).convert('L')
            if i % 4 == 0:
                mergedR.paste(image, (int((i // 4) * scale * (img_w - crop_right)), 0))
            if i % 4 == 1:
                mergedG.paste(image, (int((i // 4) * scale * (img_w - crop_right)), 0))
            if i % 4 == 2:
                mergedB.paste(image, (int((i // 4) * scale * (img_w - crop_right)), 0))
            if i % 4 == 3:
                mergedA.paste(image, (int((i // 4) * scale * (img_w - crop_right)), 0))
            os.remove(out_img) 
    merged = Image.merge('RGBA', (mergedR, mergedG, mergedB, mergedA))
    merged.save(f'../{out_file}')
    build_cov(f'../{out_file}', f'../{out_file.replace(".png", "-cov.png")}')


def build_cov(sdf_file, out_file):
    # Pre-integrated atlas: left half = sharp coverage (smoothstep of the SDF over [0.48, 0.52], keeping the
    # 0.5 border position), right half = squared SDF (glass light dispersion). VPX generates box filtered
    # mipmaps of it which directly provide the emitted light integrated over each texel footprint.
    sdf = Image.open(sdf_file).convert('RGBA')
    w, h = sdf.size
    ramp = [0] * 256
    sq = [0] * 256
    for i in range(256):
        v = i / 255.0
        t = min(max((v - 0.48) * 25.0, 0.0), 1.0)
        ramp[i] = int(t * t * (3.0 - 2.0 * t) * 255.0 + 0.5)
        sq[i] = int(v * v * 255.0 + 0.5)
    cov = sdf.point(ramp * 4)
    sdf2 = sdf.point(sq * 4)
    out = Image.new('RGBA', (2 * w, h))
    out.paste(cov, (0, 0))
    out.paste(sdf2, (w, 0))
    out.save(out_file)

if len(sys.argv) > 1 and sys.argv[1] == '--cov-only':
    # Regenerate only the coverage atlases from the existing SDF atlases (no msdfgen run)
    for sdf_file in glob.glob('../*seg*.png'):
        if '-cov' not in sdf_file:
            build_cov(sdf_file, sdf_file.replace('.png', '-cov.png'))
    sys.exit(0)

# Needed display types:
# -  7 segments
# -  7 segments and comma
# -  7 segments and dot
# -  9 segments
# -  9 segments and comma
# - 14 segments
# - 14 segments with dot
# - 14 segments with dot and comma
# - 16 segments (split top/bottom segments)

# Generic displays (made without any reference)
build_sdf('SegDisplays.svg', '16seg-generic',     4.5, 6.0, range(16),                                           '16seg.png')

# Gottlieb System 1, 80A, 80B, 3: Futaba 7, 9, 9c (made with a dot and a comma), 14dc
build_sdf('SegDisplays.svg', '7seg-gts-4digits',  4.5, 6.0, range(7),                                            '7seg-gts.png')
build_sdf('SegDisplays.svg', '9seg-gts',          4.0, 6.0, (0, 1, 2, 3, 4, 5, 6, -1, 7, 8),                     '9seg-gts.png')
build_sdf('SegDisplays.svg', '9seg-c-gts',        4.0, 6.0, range(10),                                           '9seg-c-gts.png')
build_sdf('SegDisplays.svg', '14seg-dc-gts',      1.5, 4.5, range(16),                                           '14seg-dc-gts.png')

# Bally displays: Panaplex 7, 7c, 9, 9c
build_sdf('SegDisplays.svg', '7seg-c-bally',      1.0, 4.0, range(7),                                            '7seg-bally.png')
build_sdf('SegDisplays.svg', '7seg-c-bally',      1.0, 4.0, range(8),                                            '7seg-c-bally.png')

# Williams displays: Panaplex 7, 7c, 14dc
build_sdf('SegDisplays.svg', '7seg-c-williams',   1.5, 2.5, range(7),                                            '7seg-williams.png')
build_sdf('SegDisplays.svg', '7seg-c-williams',   1.5, 2.5, range(8),                                            '7seg-c-williams.png')
build_sdf('SegDisplays.svg', '14seg-dc-williams', 1.0, 0.0, (0, 1, 2, 3, 4, 5, 6, -1, 8, 9, 10, 11, 12, 13, 14), '14seg-williams.png')
build_sdf('SegDisplays.svg', '14seg-dc-williams', 1.0, 0.0, (0, 1, 2, 3, 4, 5, 6, 15, 8, 9, 10, 11, 12, 13, 14), '14seg-d-williams.png')
build_sdf('SegDisplays.svg', '14seg-dc-williams', 1.0, 0.0, range(16),                                           '14seg-dc-williams.png')

# Atari displays: LED 7, 7c. Atari System 1 has a tighter 7seg layout than the one provided here
build_sdf('SegDisplays.svg', '7seg-c-atari',      2.0, 2.5, range(7),                                            '7seg-atari.png')
build_sdf('SegDisplays.svg', '7seg-c-atari',      2.0, 2.5, range(8),                                            '7seg-c-atari.png')
