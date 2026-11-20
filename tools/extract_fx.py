#!/usr/bin/env python3
"""
FLIPPER - extrae los objetos y efectos de las hojas de animacion (PNG con transparencia real)
y la bola metalica del arte original.

Las hojas son SOLO material para animaciones: el tablero, el logo y los personajes salen
de la imagen aprobada. Los personajes NO se recortan de las hojas.

Uso: python3 extract_fx.py <carpeta Reference> <carpeta Assets>
"""
import sys, os
from PIL import Image, ImageDraw, ImageFilter

ref, out = sys.argv[1], sys.argv[2]
sheet1 = Image.open(os.path.join(ref, "animacion", "sprite_pack_1.png")).convert("RGBA")
playfield = Image.open(os.path.join(ref, "flipper_reference_aprobada.png")).convert("RGBA")

# nombre -> (x0, y0, x1, y1) en sprite_pack_1.png
SPRITES = {
    # objetos
    "objects/pill_red.png":        (1377, 44, 1410, 81),
    "objects/pill_purple.png":     (1412, 44, 1447, 83),
    "objects/pill_orange.png":     (1452, 44, 1487, 81),
    "objects/pill_blue.png":       (445, 487, 477, 515),
    "objects/coin_a.png":          (1219, 85, 1251, 121),
    "objects/coin_b.png":          (1254, 84, 1301, 119),
    "objects/cash.png":            (1304, 82, 1351, 116),
    "objects/vinyl.png":           (1217, 237, 1321, 301),
    "objects/bone_b.png":          (1278, 302, 1330, 357),
    "objects/heart.png":           (1379, 234, 1406, 259),
    # efectos
    "effects/sparks/burst_orange.png": (1217, 121, 1257, 169),
    "effects/sparks/star_a.png":       (1258, 121, 1290, 164),
    "effects/sparks/star_b.png":       (1291, 119, 1323, 164),
    "effects/sparks/burst_big.png":    (1323, 116, 1378, 171),
    "effects/sparks/star_gold.png":    (670, 451, 716, 501),
    "effects/sparks/sparkle_magenta.png": (405, 494, 448, 541),
    "effects/sparks/explosion.png":    (627, 519, 701, 591),
    "effects/sparks/fire_burst.png":   (1000, 504, 1061, 541),
    "effects/lightning/bolt_a.png":    (1377, 116, 1412, 173),
    "effects/lightning/bolt_b.png":    (1452, 82, 1516, 158),
    "effects/smoke/puff_a.png":        (1397, 179, 1443, 236),
    "effects/smoke/puff_b.png":        (1366, 301, 1420, 358),
    "effects/smoke/puff_c.png":        (902, 464, 934, 494),
    "effects/smoke/wisp.png":          (1259, 161, 1303, 236),
    "effects/smoke/swirl.png":         (1087, 459, 1186, 549),
    "effects/rainbow/arc_wide.png":    (507, 536, 606, 591),
    "effects/rainbow/arc_small.png":   (507, 461, 581, 531),
}

def clean(img):
    """Quita el ruido de alfa muy bajo que dejan las hojas alrededor de cada objeto."""
    r, g, b, a = img.split()
    a = a.point(lambda v: 0 if v < 28 else min(255, int((v - 28) * 255 / 227)))
    return Image.merge("RGBA", (r, g, b, a))

for name, box in SPRITES.items():
    path = os.path.join(out, name)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    clean(sheet1.crop(box)).save(path)

# bola metalica: la del arte original (no la de las hojas)
cx, cy, r = 565, 769, 22
ball = playfield.crop((cx - r, cy - r, cx + r, cy + r))
m = Image.new("L", (r * 8, r * 8), 0)
ImageDraw.Draw(m).ellipse((6, 6, r * 8 - 7, r * 8 - 7), fill=255)
ball.putalpha(m.resize((r * 2, r * 2), Image.LANCZOS))
os.makedirs(os.path.join(out, "effects", "ball"), exist_ok=True)
ball.save(os.path.join(out, "effects", "ball", "ball_chrome.png"))

print(len(SPRITES) + 1, "sprites")
