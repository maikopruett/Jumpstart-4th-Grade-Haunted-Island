#!/usr/bin/env python3
"""List (or dump) the live functions of a porting unit from re/units.json.

usage: unit.py UNIT            list functions with sizes
       unit.py UNIT --dump     print the decompiled code of all of them
"""
import json, os, re, sys
here = os.path.dirname(os.path.abspath(__file__))
root = os.path.join(here, '..')
units = json.load(open(os.path.join(root, 'units.json')))
src = open(os.path.join(root, 'out', '4G.EXE.c')).read()
dead = set(json.load(open(os.path.join(root, 'out', 'partition.json')))['unreached'])
live_extra = {'FUN_0040109d','FUN_004010aa','FUN_004010b7','FUN_004010c4','FUN_004010d1','FUN_004010de',
              'FUN_004010eb','FUN_004010f8','FUN_00401105','FUN_00401112','FUN_0040111f','FUN_0040107c',
              'FUN_00402624','FUN_00402638','FUN_00402660','FUN_00402450'}
dead -= live_extra
u = sys.argv[1]
for part in re.split(r'(?m)^// ==== ', src)[1:]:
    name, rest = part.split(' @ ', 1)
    a = int(rest[:8], 16)
    if not any(int(lo, 16) <= a < int(hi, 16) for lo, hi in units[u]): continue
    if 0x40189b <= a < 0x402450: continue
    if name in dead:
        if '--dump' not in sys.argv: print(f"{name}  (dead code, skip)")
        continue
    if '--dump' in sys.argv: print('// ==== ' + part)
    else: print(f"{name}  {part.count(chr(10))} lines")
