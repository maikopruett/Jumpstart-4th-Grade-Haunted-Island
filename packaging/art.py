#!/usr/bin/env python3
"""Builds the installer artwork from images extracted from the user's game
data by packtool (packaging/packtool.cpp): the game's 32x32 icon and the
tombstone screen.

    art.py <icon32.png> <tombstone.png> <out_dir>

Writes into out_dir:
    AppIcon.iconset/             macOS icon sizes (for iconutil)
    AppIcon.ico                  Windows icon
    icon-256.png, icon-512.png   Linux icons
    background.png, @2x          macOS disk image window
    nsis-welcome.bmp             Windows installer welcome/finish side image
    nsis-header.bmp              Windows installer page header
"""
import os
import sys

from PIL import Image, ImageDraw, ImageFilter, ImageFont


def tile_icon(art, size, body, radius, shadow):
    """The original pixel-art icon on a rounded square."""
    master = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    off = (size - body) // 2
    if shadow:
        s = Image.new('L', (size, size), 0)
        ImageDraw.Draw(s).rounded_rectangle((off, off + 12, off + body, off + body + 12), radius, fill=110)
        master.paste((0, 0, 0, 255), (0, 0), s.filter(ImageFilter.GaussianBlur(14)))
    tile = art.resize((body, body), Image.NEAREST)
    mask = Image.new('L', (body, body), 0)
    ImageDraw.Draw(mask).rounded_rectangle((0, 0, body - 1, body - 1), radius, fill=255)
    master.paste(tile, (off, off), mask)
    # Thin dark rim so the tile reads on light backgrounds.
    rim = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    ImageDraw.Draw(rim).rounded_rectangle((off, off, off + body - 1, off + body - 1), radius,
                                           outline=(0, 0, 40, 160), width=6)
    master.alpha_composite(rim)
    return master


def icons(src, out):
    art = Image.open(src).convert('RGBA')
    # macOS: Big Sur style grid (824/1024 body with a shadow; 800 keeps the
    # 32px art at an integer 25x scale).
    mac = tile_icon(art, 1024, 800, 180, True)
    iconset = os.path.join(out, 'AppIcon.iconset')
    os.makedirs(iconset, exist_ok=True)
    for s in (16, 32, 128, 256, 512):
        for scale in (1, 2):
            name = f'icon_{s}x{s}' + ('@2x' if scale == 2 else '') + '.png'
            mac.resize((s * scale, s * scale), Image.LANCZOS).save(os.path.join(iconset, name))
    # Windows and Linux: fuller tile, no shadow.
    flat = tile_icon(art, 1024, 960, 150, False)
    flat.save(os.path.join(out, 'AppIcon.ico'), sizes=[(16, 16), (24, 24), (32, 32), (48, 48), (64, 64),
                                                     (128, 128), (256, 256)])
    for s in (256, 512):
        flat.resize((s, s), Image.LANCZOS).save(os.path.join(out, f'icon-{s}.png'))


def find_font(names, size):
    for n in names:
        for d in ('/System/Library/Fonts/Supplemental', '/System/Library/Fonts', '/Library/Fonts'):
            p = os.path.join(d, n)
            if os.path.exists(p):
                return ImageFont.truetype(p, size)
    return ImageFont.load_default(size)


def carve(img, xy, text, font, anchor='mm'):
    """Text that looks chiselled into the stone: dark groove, light lower edge."""
    k = max(1, img.width // 640)
    layer = Image.new('RGBA', img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    x, y = xy
    d.text((x + k, y + k), text, font=font, fill=(235, 235, 245, 150), anchor=anchor)
    d.text((x, y), text, font=font, fill=(38, 38, 52, 235), anchor=anchor)
    img.alpha_composite(layer)


def arrow(img, x0, x1, y):
    k = img.width / 640
    layer = Image.new('RGBA', img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    w, head = 7 * k, 16 * k
    for dx, dy, col in ((k, k, (235, 235, 245, 150)), (0, 0, (38, 38, 52, 235))):
        d.line((x0 + dx, y + dy, x1 - head + dx, y + dy), fill=col, width=round(w))
        d.polygon([(x1 + dx, y + dy), (x1 - head * 1.4 + dx, y - head + dy), (x1 - head * 1.4 + dx, y + head + dy)],
                  fill=col)
    img.alpha_composite(layer)


def dmg_background(base, out):
    """640x480 window: the tombstone with install instructions carved into it.
    Icons go at (245, 225) and (395, 225) (see mac/dmg_settings.py)."""
    for scale, name in ((1, 'background.png'), (2, 'background@2x.png')):
        img = base.resize((640 * scale, 480 * scale), Image.LANCZOS)
        title = find_font(['Herculanum.ttf', 'Papyrus.ttc'], 25 * scale)
        hint = find_font(['Herculanum.ttf', 'Papyrus.ttc'], 15 * scale)
        carve(img, (320 * scale, 118 * scale), 'JumpStart 4th Grade', title)
        carve(img, (320 * scale, 146 * scale), 'Haunted Island', hint)
        carve(img, (320 * scale, 330 * scale), 'Drag the game to Applications', hint)
        arrow(img, 298 * scale, 342 * scale, 225 * scale)
        img.convert('RGB').save(os.path.join(out, name))


def nsis_images(base, out):
    # Welcome/finish page: a tall slice centred on the tombstone, 164x314.
    w = round(480 * 164 / 314)
    side = base.crop((320 - w // 2, 0, 320 - w // 2 + w, 480)).resize((164, 314), Image.LANCZOS).convert('RGBA')
    carve(side, (82, 96), 'JumpStart', find_font(['Herculanum.ttf', 'Papyrus.ttc'], 17))
    carve(side, (82, 116), '4th Grade', find_font(['Herculanum.ttf', 'Papyrus.ttc'], 14))
    side.convert('RGB').save(os.path.join(out, 'nsis-welcome.bmp'))
    # Page header: the purple trees from the top right of the scene, 150x57.
    head = base.crop((500, 0, 640, 53)).resize((150, 57), Image.LANCZOS)
    head.convert('RGB').save(os.path.join(out, 'nsis-header.bmp'))


if __name__ == '__main__':
    if len(sys.argv) != 4:
        sys.exit(__doc__)
    icon_src, scene_src, out_dir = sys.argv[1:]
    os.makedirs(out_dir, exist_ok=True)
    icons(icon_src, out_dir)
    scene = Image.open(scene_src).convert('RGBA')
    dmg_background(scene, out_dir)
    nsis_images(scene, out_dir)
