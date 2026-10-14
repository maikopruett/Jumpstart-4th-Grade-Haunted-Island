#!/usr/bin/env python3
"""Compare cdecl call-site stack cleanup with Ghidra's parameter counts."""
import re, collections
asm = open('re/out/4G.asm').read().split('\n')
src = open('re/out/4G.EXE.c').read()
nparams = {}
for part in re.split(r'(?m)^// ==== ', src)[1:]:
    name, rest = part.split(' @ ', 1)
    addr = int(rest[:8], 16)
    body = part.split('\n', 1)[1]
    m = re.search(r'\b' + re.escape(name) + r'\s*\(([^)]*)\)', body)
    if not m: continue
    args = m.group(1).strip()
    n = 0 if args in ('', 'void') else args.count(',') + 1
    if '...' in args: n = -1
    nparams[addr] = (name, n)
obs = collections.defaultdict(collections.Counter)
for i, line in enumerate(asm):
    m = re.search(r'call   0x([0-9a-f]+)$', line)
    if not m: continue
    tgt = int(m.group(1), 16)
    nxt = asm[i + 1] if i + 1 < len(asm) else ''
    if re.search(r'pop    ecx$', nxt): k = 1
    else:
        m2 = re.search(r'add    esp,0x([0-9a-f]+)$', nxt)
        k = int(m2.group(1), 16) // 4 if m2 else 0
    obs[tgt][k] += 1
for tgt, c in sorted(obs.items()):
    if tgt not in nparams or tgt >= 0x41c62c: continue
    name, n = nparams[tgt]
    if n < 0: continue
    k = max(c)  # most args observed
    if k != n and not (k == 0 and n > 0):
        print(f"{tgt:#x} {name}: ghidra={n} callsites={dict(c)}")
