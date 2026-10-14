#!/usr/bin/env python3
"""Partition game functions (0x401000..0x41c62c) by module reachability."""
import re, sys, collections, json
src = open('re/out/4G.EXE.c').read()
funcs = {}
for part in re.split(r'(?m)^// ==== ', src)[1:]:
    name, rest = part.split(' @ ', 1)
    addr = int(rest[:8], 16)
    size = int(re.search(r'size (\d+)', rest).group(1))
    body = part.split('\n', 1)[1]
    funcs[name] = (addr, size, body)
GAME_END = 0x41c62c
def is_game(n): return n in funcs and funcs[n][0] < GAME_END
calls = {n: set(c for c in re.findall(r'\b(FUN_[0-9a-f]{8}|[A-Za-z_][A-Za-z0-9_]*)\s*\(', funcs[n][2]) if is_game(c) and c != n) for n in funcs if is_game(n)}
# also references to function addresses (callbacks): FUN_xxx without call parens
refs = {n: set(c for c in re.findall(r'\b(FUN_[0-9a-f]{8})\b', funcs[n][2]) if is_game(c) and c != n) for n in calls}
entries = {
 'init': ['FUN_00402674','FUN_00403080'],
 'm01_signin': ['FUN_00404f80','FUN_00405719'], 'm02': ['FUN_0040415e','FUN_00404bdb'],
 'm03': ['FUN_004084c0','FUN_004085f4'], 'm04': ['FUN_00406890','FUN_004069e8'],
 'm05': ['FUN_004133c8','FUN_004134db'], 'm06': ['FUN_00411524','FUN_004115ee'],
 'm07': ['FUN_0040bccb','FUN_0040bcf8'], 'm08': ['FUN_0040f697','FUN_0040f6bc'],
 'm09': ['FUN_0040f7bc','FUN_0040fcac'], 'm10': ['FUN_0040ce63','FUN_0040ce90'],
 'm11': ['FUN_00418494','FUN_004184b9'], 'm13': ['FUN_004186af','FUN_00418a61'],
 'm14': ['FUN_00419a56','FUN_0041a136'], 'm15': ['FUN_0041a6e8','FUN_0041a7df'],
 'm17': ['FUN_0041a8c8','FUN_0041a93d'], 'm18': ['FUN_0041bab2'], 'm19': ['FUN_0041935e'],
 'm22': ['FUN_0040112c','FUN_004011b6'], 'quit': ['FUN_00402a3a','FUN_00402764','FUN_004193ec','FUN_004027ae','FUN_00402610'],
}
reach = {}
for m, es in entries.items():
    seen = set(); stack = [e for e in es if e in calls]
    while stack:
        f = stack.pop()
        if f in seen: continue
        seen.add(f)
        stack.extend(refs.get(f, ()))
    reach[m] = seen
owner = collections.defaultdict(set)
for m, s in reach.items():
    for f in s: owner[f].add(m)
mods = collections.defaultdict(list)
for f in calls:
    o = owner.get(f)
    key = 'unreached' if not o else (next(iter(o)) if len(o) == 1 else 'common')
    mods[key].append(f)
summary = {}
for k in sorted(mods):
    fl = sorted(mods[k], key=lambda n: funcs[n][0])
    lines = sum(funcs[n][2].count('\n') for n in fl)
    summary[k] = {'n': len(fl), 'lines': lines, 'range': [hex(funcs[fl[0]][0]), hex(funcs[fl[-1]][0])]}
    print(f"{k:12} funcs={len(fl):4} lines={lines:6} range {hex(funcs[fl[0]][0])}-{hex(funcs[fl[-1]][0])}")
json.dump({k: sorted(v, key=lambda n: funcs[n][0]) for k, v in mods.items()}, open('re/out/partition.json', 'w'), indent=1)
