#!/usr/bin/env python3
"""Decode KA 'P800' 8-bit images to PNG (for verifying the format).

Header: 'P800' u16 w, u16 h, u8 transparent, u8 encoding, u16 pad.
encoding 0: raw w*h; 1: raw column-major; 2: row RLE; 3: column RLE.
RLE (types 2/3): +0xc u32 datasize, +0x10 u32 offset[n] (relative to +0xc);
each run byte b: b&0x80 -> repeat next byte (b&0x7f) times, else b literals.

usage: p800.py FILE.DF IMAGE_ID PALETTE_ID OUT.png
"""
import os, struct, sys, zlib
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import kadf


def decode(img):
    assert img[:4] == b'P800'
    w, h, transp, enc = struct.unpack_from('<HHBB', img, 4)
    px = bytearray(w * h)
    if enc == 0:
        px[:] = img[12:12 + w * h]
    elif enc == 1:
        for x in range(w):
            for y in range(h):
                px[y * w + x] = img[12 + x * h + y]
    else:
        n = h if enc == 2 else w
        length = w if enc == 2 else h
        for line in range(n):
            p = 12 + struct.unpack_from('<I', img, 16 + 4 * line)[0]
            i = 0
            while i < length:
                b = img[p]; p += 1
                if b & 0x80:
                    run, v = b & 0x7f, img[p]; p += 1
                    vals = [v] * run
                else:
                    vals = img[p:p + b]; p += b
                for v in vals:
                    if enc == 2:
                        px[line * w + i] = v
                    else:
                        px[i * w + line] = v
                    i += 1
    return w, h, transp, px


def write_png(path, w, h, rgb):
    raw = b''.join(b'\0' + rgb[y * w * 3:(y + 1) * w * 3] for y in range(h))
    def chunk(t, d):
        return struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d) & 0xffffffff)
    open(path, 'wb').write(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0))
                           + chunk(b'IDAT', zlib.compress(raw)) + chunk(b'IEND', b''))


if __name__ == '__main__':
    a = kadf.Archive(sys.argv[1])
    w, h, t, px = decode(a.get(int(sys.argv[2], 0)))
    pal = a.get(int(sys.argv[3], 0))[:768]
    rgb = bytes(c for i in px for c in pal[i * 3:i * 3 + 3])
    write_png(sys.argv[4], w, h, rgb)
    print(w, h, 'transparent', t)
