#!/usr/bin/env python3
"""Survey resources in KA .DF archives: per-type counts, ID ranges, flags."""
import struct, sys, collections

def entries(path):
    d = open(path, 'rb').read()
    magic, base, count = struct.unpack_from('<III', d, 0)
    assert magic == 0x4644, path
    data0 = 12 + 4 * count
    offs = struct.unpack_from('<%dI' % count, d, 12)
    for i in range(count):
        start = offs[i] + data0
        end = (offs[i + 1] + data0) if i + 1 < count else len(d)
        if end - start < 16:
            continue
        tag, comp, hdr, size = struct.unpack_from('<4sIII', d, start)
        yield base + i, tag, comp, hdr, size, start, end - start - 16

for path in sys.argv[1:]:
    stats = collections.OrderedDict()
    for rid, tag, comp, hdr, size, start, clen in entries(path):
        s = stats.setdefault(tag, [0, rid, rid, collections.Counter(), 0])
        s[0] += 1; s[1] = min(s[1], rid); s[2] = max(s[2], rid); s[3][(comp, hdr)] += 1; s[4] += size
    print(path)
    for tag, (n, lo, hi, flags, tot) in stats.items():
        print('  %-8r n=%5d ids %5d-%5d  flags=%s  total=%d' % (tag, n, lo, hi, dict(flags), tot))
