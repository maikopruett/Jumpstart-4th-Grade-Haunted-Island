#!/usr/bin/env python3
"""Reader for Knowledge Adventure .DF archives (JumpStart 4th Grade, 1997).

Archive layout (little endian):
    u32 magic 'DF\\0\\0'   u32 base_id   u32 count   u32 offset[count]
    data area begins at 12 + 4*count; offset[i] is relative to it.
Each resource: 16-byte header {u32 junk, u32 compressed, u32 hdrsize(16),
u32 size} followed by payload. Compressed payload is a 'COMP' container:
    'COMP' u32 total, then chunks {u32 usize, u32 csize, csize bytes};
    a chunk with usize == csize is stored raw, otherwise it is a
    Huffman+LZ77 stream (see decompress_chunk). Mirrors 4G.EXE 0x4272c4,
    0x425980 and the hand-written asm at 0x401a44-0x40207a.

usage: kadf.py list FILE.DF | kadf.py extract FILE.DF ID OUT
"""
import struct, sys

LIT_SYMS = 0x11e      # 0..255 literals, 0x100..0x11c lengths 3..31, 0x11d long length
DIST_SYMS = 0x80
DEFAULT_DIST = bytes([0x02, 0x01, 0x01, 0x03, 0x06, 0x0d, 0x1f, 0x4e, 0x00])


class Bits:
    def __init__(self, data, pos):
        self.d, self.byte, self.bit = data, pos, 0

    def get(self, n):
        v = 0
        for _ in range(n):
            b = self.d[self.byte] if self.byte < len(self.d) else 0
            v = (v << 1) | ((b >> (7 - self.bit)) & 1)
            self.bit += 1
            if self.bit == 8:
                self.bit, self.byte = 0, self.byte + 1
        return v


def build_tree(lengths):
    """Recreate the asm's tree builder: walks depth-first, giving each leaf
    position the next symbol (in index order) of the current target length."""
    nsyms = len(lengths)
    left, right = {}, {}
    st = {'depth': 0, 'L': 1, 'next': nsyms, 'idx': -1}

    def build():
        if st['L'] == st['depth']:
            while True:
                st['idx'] += 1
                if st['idx'] >= nsyms:
                    break
                if lengths[st['idx']] == st['L']:
                    return st['idx']
            st['idx'] = -1
            st['L'] += 1
            if st['L'] > 32:
                raise ValueError('bad huffman lengths')
        node = st['next']; st['next'] += 1
        st['depth'] += 1
        left[node] = build()
        right[node] = build()
        st['depth'] -= 1
        return node

    root = build()
    return root, left, right


class Decoder:
    def __init__(self, lengths, single=None):
        self.single = single
        if single is None:
            self.root, self.left, self.right = build_tree(lengths)
            self.n = len(lengths)

    def decode(self, bits):
        if self.single is not None:
            return self.single
        node = self.root
        while node >= self.n:
            node = self.right[node] if bits.get(1) else self.left[node]
        return node


def read_lit_table(bits):
    lens = [0] * LIT_SYMS
    for i in range(3):
        lens[i] = bits.get(4) + 1 if bits.get(1) else 0
    if lens[0] == 1 and lens[1] == 1 and lens[2] == 1:
        return Decoder(None, single=bits.get(9))
    for i in range(3, LIT_SYMS):
        lens[i] = bits.get(4) + 1 if bits.get(1) else 0
    return Decoder(lens)


def read_dist_table(bits):
    if not bits.get(1):
        lens, ln, k = [], DEFAULT_DIST[0], 1
        for sym in range(DIST_SYMS):
            while sym == DEFAULT_DIST[k]:
                ln += 1; k += 1
            lens.append(ln)
        return Decoder(lens)
    lens = [bits.get(4) for _ in range(3)] + [0] * (DIST_SYMS - 3)
    if lens[0] == 1 and lens[1] == 1 and lens[2] == 1:
        return Decoder(None, single=bits.get(7))
    for i in range(3, DIST_SYMS):
        lens[i] = bits.get(4)
    return Decoder(lens)


def decompress_chunk(src):
    size = struct.unpack_from('<I', src, 0)[0] & 0x3fffffff
    bits = Bits(src, 6)
    bits.get(1)
    lit = read_lit_table(bits)
    dist = read_dist_table(bits)
    out = bytearray()
    while len(out) < size:
        sym = lit.decode(bits)
        if sym < 0x100:
            out.append(sym)
            continue
        if sym == 0x11d:
            sym += bits.get(8)
        length = sym - 0xfd
        d = ((dist.decode(bits) << 6) | bits.get(6)) + 1
        for _ in range(length):
            out.append(out[-d])
    return bytes(out)


def decompress(buf):
    if len(buf) < 4 or buf[:4] != b'COMP':
        return buf
    total = struct.unpack_from('<i', buf, 4)[0]
    pos, out = 8, bytearray()
    while len(out) < total:
        usize, csize = struct.unpack_from('<II', buf, pos)
        pos += 8
        chunk = buf[pos:pos + csize]
        out += chunk if usize == csize else decompress_chunk(chunk)
        pos += csize
    return bytes(out)


class Archive:
    def __init__(self, path):
        self.d = open(path, 'rb').read()
        magic, self.base, self.count = struct.unpack_from('<III', self.d, 0)
        assert magic == 0x4644, 'not a DF archive'
        self.data0 = 12 + 4 * self.count
        self.offs = struct.unpack_from('<%dI' % self.count, self.d, 12)

    def ids(self):
        for i in range(self.count):
            if self._span(i)[1] >= 16:
                yield self.base + i

    def _span(self, i):
        start = self.offs[i] + self.data0
        end = (self.offs[i + 1] + self.data0) if i + 1 < self.count else len(self.d)
        return start, end - start

    def get(self, rid):
        i = rid - self.base
        if not 0 <= i < self.count:
            return None
        start, n = self._span(i)
        if n < 16:
            return None
        _, comp, _, size = struct.unpack_from('<IIII', self.d, start)
        payload = self.d[start + 16:start + n]
        if comp & 0xff == 1:
            payload = decompress(payload)
        return payload[:size]


if __name__ == '__main__':
    cmd = sys.argv[1]
    a = Archive(sys.argv[2])
    if cmd == 'list':
        for rid in a.ids():
            print(rid)
    elif cmd == 'extract':
        open(sys.argv[4], 'wb').write(a.get(int(sys.argv[3])))
