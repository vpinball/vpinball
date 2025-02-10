# This little python script generates the signed distance field atlases used for rendering segment displays
# It requires https://github.com/Chlumsky/msdfgen to be in the path

# pip install Pillow
# pip install lxml

from lxml import etree
from PIL import Image
import subprocess
import os

img_size = 128

def extract_single_path(svg_file, group_id, path_index, out_img):
    parser = etree.XMLParser(remove_blank_text=False)
    tree = etree.parse(svg_file, parser)
    root = tree.getroot()
    namespace = {'svg': 'http://www.w3.org/2000/svg'}
    path_found = False
    for group in root.findall('svg:g', namespaces=namespace):
        if group.attrib.get('id') != 'Segments':
            root.remove(group)
        else:
            for subgroup in group.findall('svg:g', namespaces=namespace):
                if subgroup.attrib.get('id') != group_id:
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
        subprocess.run(["msdfgen.exe", "sdf", "-svg" , tmp_file, "-o", out_img, "-pxrange", "32.0", "-translate", "8.0", "4.0", "-scale", "4.0", "-size", f"{img_size}", f"{img_size}"])
        #subprocess.run(["msdfgen.exe", "sdf", "-svg" , tmp_file, "-o", out_img, "-pxrange", "32.0", "-translate", "8.0", "4.0", "-scale", "4.0", "-size", f"{img_size}", f"{img_size}", "-testrender", f"render-{out_img}", "1024", "1024"])
        os.remove(tmp_file) 
    print(f"{group_id}-Seg{path_index}: {path_found}")
    return path_found


def build_sdf(svg_file, group_id, segs, out_file):
    mergedR = Image.new('L', (4 * img_size, img_size))
    mergedG = Image.new('L', (4 * img_size, img_size))
    mergedB = Image.new('L', (4 * img_size, img_size))
    mergedA = Image.new('L', (4 * img_size, img_size))
    for i, idx in enumerate(segs):
        out_img = f"{group_id}-Seg{i}.png"
        if idx >= 0 and extract_single_path(svg_file, group_id, idx, out_img):
            image = Image.open(out_img).convert('L')
            if i % 4 == 0:
                mergedR.paste(image, ((i // 4) * img_size, 0))
            if i % 4 == 1:
                mergedG.paste(image, ((i // 4) * img_size, 0))
            if i % 4 == 2:
                mergedB.paste(image, ((i // 4) * img_size, 0))
            if i % 4 == 3:
                mergedA.paste(image, ((i // 4) * img_size, 0))
            os.remove(out_img) 
    merged = Image.merge('RGBA', (mergedR, mergedG, mergedB, mergedA))
    merged.save(f'../{out_file}')


build_sdf('SegDisplays.svg', '7seg-clock', range(7),                                               '7seg-gts1.png')
build_sdf('SegDisplays.svg', '7seg-dc',    range(7),                                               '7seg.png')
build_sdf('SegDisplays.svg', '7seg-dc',    (0, 1, 2, 3, 4, 5, 6, 7),                               '7seg-c.png')
build_sdf('SegDisplays.svg', '7seg-dc',    (0, 1, 2, 3, 4, 5, 6, 8),                               '7seg-d.png')
build_sdf('SegDisplays.svg', '10seg-dc',   (0, 1, 2, 3, 4, 5, 6, 7, 8, 9),                         '9seg.png')
build_sdf('SegDisplays.svg', '10seg-dc',   range(10),                                              '9seg-c.png')
build_sdf('SegDisplays.svg', '14seg-dc',   (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15), '14seg.png')
build_sdf('SegDisplays.svg', '14seg-dc',   (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15), '14seg-d.png')
build_sdf('SegDisplays.svg', '14seg-dc',   range(16),                                              '14seg-dc.png')
build_sdf('SegDisplays.svg', '16seg',      range(16),                                              '16seg.png')

