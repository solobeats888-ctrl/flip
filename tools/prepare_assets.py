#!/usr/bin/env python3
"""
FLIPPER - preparacion de assets a partir de la imagen aprobada (1024x1536).

Genera:
  Assets/Background/main_playfield.jpg   fondo sin textos horneados (los textos los dibuja JUCE)
  Assets/Controls/knob_macro_N.png       cada perilla macro recortada (gira en JUCE)
  Assets/Controls/knob_character.png     perilla del panel PERSONAJE
  Assets/Controls/mode_led_on.png        LED encendido (de JUGAR)
  Assets/Controls/mode_led_off.png       LED apagado (de DISEÑO)
  Assets/Characters/<id>_character.png   retratos recortados del playfield

Los retratos son recortes de la referencia aprobada (definitivos).

Uso: python3 prepare_assets.py <referencia.png> <carpeta Assets>
"""
import sys, os
import numpy as np
import cv2
from PIL import Image, ImageDraw, ImageFilter

ref_path, out = sys.argv[1], sys.argv[2]
for sub in ("Background", "Controls", "Characters"):
    os.makedirs(os.path.join(out, sub), exist_ok=True)

src = Image.open(ref_path).convert("RGB")
assert src.size == (1024, 1536), src.size
arr = np.array(src)
bgr = cv2.cvtColor(arr, cv2.COLOR_RGB2BGR)
hsv = cv2.cvtColor(bgr, cv2.COLOR_BGR2HSV)
luma = cv2.cvtColor(bgr, cv2.COLOR_BGR2GRAY).astype(np.int32)
H, W = luma.shape

mask_all = np.zeros((H, W), np.uint8)

def rect_mask(x0, y0, x1, y1):
    m = np.zeros((H, W), np.uint8)
    m[y0:y1, x0:x1] = 1
    return m

def add(mask, dil=2):
    global mask_all
    k = np.ones((3, 3), np.uint8)
    m = cv2.dilate(mask.astype(np.uint8), k, iterations=dil)
    mask_all = np.maximum(mask_all, m)

# --- textos oscuros sobre placas amarillas (nombres de motores) -------------
plates = [  # caja del texto (x0,y0,x1,y1)
    (250, 426, 342, 466),   # MINIMOOG
    (444, 428, 549, 458),   # PROPHET-5
    (654, 432, 760, 473),   # JUPITER-8
    (249, 723, 371, 764),   # JUNO-60/106
    (474, 671, 541, 702),   # CS-80
    (673, 729, 721, 764),   # DX7
    (251, 988, 286, 1019),  # M1
    (481, 1074, 590, 1112), # ARP 2600
    (728, 1099, 792, 1134), # VCS3
]
# Las placas del playfield conservan el nombre del sintetizador tal cual esta en el arte
# aprobado (el nombre del personaje se muestra solo en el panel PERSONAJE).
plate_fill = []

# --- textos claros sobre paneles oscuros ------------------------------------
light_text = [
    (843, 252, 982, 299),    # ENGINE MIX
    (876, 322, 950, 700),    # nombres en ENGINE MIX
    (876, 747, 992, 780),    # CHARACTER
    (902, 997, 972, 1029),   # DIRTY
    (98, 302, 160, 351),     # PLAY
    (91, 394, 162, 434),     # DESIGN
    (98, 478, 140, 516),     # LAB
]
for b in light_text:
    add(rect_mask(*b) & (luma > 95), dil=2)

# macro labels (amarillo / crema sobre negro)
add(rect_mask(40, 1412, 800, 1448) & (luma > 80), dil=2)
# VARIATIONS / FAVORITE (crema sobre negro)
add(rect_mask(886, 1410, 1002, 1438) & (luma > 90), dil=2)

# subtitulo VINTAGE SYNTHESIS ENGINE (amarillo saturado)
yel = (hsv[..., 0] >= 12) & (hsv[..., 0] <= 35) & (hsv[..., 1] > 110) & (hsv[..., 2] > 120)
add(rect_mask(812, 12, 932, 122) & yel, dil=2)

clean = cv2.inpaint(bgr, mask_all * 255, 6, cv2.INPAINT_TELEA)

rng = np.random.default_rng(1974)
for (x0, y0, x1, y1), m in plate_fill:
    pad = 6
    bx0, by0, bx1, by1 = x0 - pad, y0 - pad, x1 + pad, y1 + pad
    region = clean[by0:by1, bx0:bx1]
    rm = m[by0:by1, bx0:bx1].astype(bool)
    rl = luma[by0:by1, bx0:bx1]
    donors = region[(~rm) & (rl > 140)]
    if len(donors) == 0:
        continue
    picks = donors[rng.integers(0, len(donors), rm.sum())]
    region[rm] = picks
    blurred = cv2.medianBlur(region, 3)
    region[rm] = blurred[rm]
    clean[by0:by1, bx0:bx1] = region

# GENERATE: texto negro sobre boton amarillo -> rellenar con amarillo del boton
gen = rect_mask(815, 1409, 880, 1437) & (luma < 110)
clean = cv2.inpaint(clean, cv2.dilate(gen.astype(np.uint8), np.ones((3, 3), np.uint8), 2) * 255, 5, cv2.INPAINT_TELEA)

# mode LED: el fondo queda con los tres LEDs apagados (el encendido lo pinta JUCE)
off_led = Image.fromarray(arr).crop((61 - 22, 412 - 22, 61 + 22, 412 + 22))
clean_rgb = Image.fromarray(cv2.cvtColor(clean, cv2.COLOR_BGR2RGB))
big = off_led.resize((58, 58), Image.LANCZOS)
m = Image.new("L", big.size, 0); ImageDraw.Draw(m).ellipse((1, 1, 56, 56), fill=255)
m = m.filter(ImageFilter.GaussianBlur(1.2))
clean_rgb.paste(big, (66 - 29, 321 - 29), m)

clean_rgb.save(os.path.join(out, "Background", "main_playfield.jpg"), quality=92, subsampling=0)

# --- sprites ---------------------------------------------------------------
def circle_sprite(cx, cy, r, name, size=None, feather=1.0):
    crop = src.crop((cx - r, cy - r, cx + r, cy + r))
    if size: crop = crop.resize((size, size), Image.LANCZOS)
    s = crop.size[0]
    m = Image.new("L", (s * 4, s * 4), 0)
    ImageDraw.Draw(m).ellipse((2, 2, s * 4 - 3, s * 4 - 3), fill=255)
    m = m.resize((s, s), Image.LANCZOS)
    rgba = crop.convert("RGBA"); rgba.putalpha(m)
    rgba.save(os.path.join(out, "Controls", name))

macro_x = [85, 183, 285, 385, 479, 577, 668, 758]
for i, x in enumerate(macro_x):
    circle_sprite(x, 1376, 27, f"knob_macro_{i+1}.png")
circle_sprite(933, 1093, 30, "knob_character.png")
circle_sprite(66, 321, 24, "mode_led_on.png")
circle_sprite(61, 412, 21, "mode_led_off.png")

# --- retratos ---------------------------------------------------
# (id, centro de la cara, alto del recorte). Aspecto = ovalo del panel PERSONAJE (58x74)
portraits = [
    ("minimoog", (292, 345), 190),
    ("prophet5", (497, 300), 220),
    ("jupiter8", (705, 345), 190),
    ("juno",     (300, 600), 250),
    ("cs80",     (505, 605), 190),
    ("dx7",      (705, 615), 250),
    ("m1",       (225, 880), 230),
    ("arp2600",  (530, 935), 260),
]
aspect = 58.0 / 74.0
for pid, (cx, cy), h in portraits:
    w = int(h * aspect)
    box = (cx - w // 2, cy - h // 2, cx + w // 2, cy + h // 2)
    src.crop(box).resize((232, 296), Image.LANCZOS).save(os.path.join(out, "Characters", f"{pid}_character.png"))
# VCS3: la referencia ya trae su retrato dibujado en el panel CHARACTER
src.crop((934 - 58, 889 - 74, 934 + 58, 889 + 74)).resize((232, 296), Image.LANCZOS).save(
    os.path.join(out, "Characters", "vcs3_character.png"))

Image.fromarray(mask_all * 255).save("/tmp/flipper_inpaint_mask.png")
print("ok")
