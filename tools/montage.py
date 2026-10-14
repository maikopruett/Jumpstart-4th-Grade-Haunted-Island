#!/usr/bin/env python3
"""Tile PNG snapshots into one image (half size) with labels."""
import sys
from PIL import Image, ImageDraw
out, files = sys.argv[1], sys.argv[2:]
cols = 3 if len(files) > 4 else 2
rows = (len(files) + cols - 1) // cols
W, H = 320, 240
sheet = Image.new('RGB', (cols * W, rows * (H + 14)), (40, 40, 40))
d = ImageDraw.Draw(sheet)
for i, f in enumerate(files):
    im = Image.open(f).convert('RGB').resize((W, H))
    x, y = (i % cols) * W, (i // cols) * (H + 14)
    sheet.paste(im, (x, y + 14))
    d.text((x + 4, y + 1), f.split('/')[-1], fill=(255, 255, 0))
sheet.save(out)
