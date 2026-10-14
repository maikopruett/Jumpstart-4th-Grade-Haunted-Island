#!/usr/bin/env python3
"""Print decompiled functions from re/out/4G.EXE.c.

usage: fn.py NAME_OR_ADDR [...]      print functions
       fn.py -r DEPTH NAME           print function and callees to DEPTH
       fn.py -x NAME                 list callers of NAME
"""
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, '..', 'out', '4G.EXE.c')

def load():
    src = open(SRC).read()
    funcs = {}
    for part in re.split(r'(?m)^// ==== ', src)[1:]:
        name, rest = part.split(' @ ', 1)
        addr = rest[:8]
        body = '// ==== ' + part
        funcs[name] = body
        funcs[addr] = body
        funcs['0x' + addr.lstrip('0')] = body
    return funcs

def key(funcs, k):
    for cand in (k, 'FUN_' + k.replace('0x', '').rjust(8, '0'), k.replace('0x', '').rjust(8, '0')):
        if cand in funcs:
            return cand
    return None

def main():
    funcs = load()
    args = sys.argv[1:]
    if args and args[0] == '-x':
        target = args[1]
        seen = set()
        for k, body in funcs.items():
            if k.startswith('FUN_') or not re.match(r'^[0-9a-f]{8}$|^0x', k):
                if re.search(r'\b%s\b' % re.escape(target), body.split('\n', 1)[1]) and body not in seen:
                    seen.add(body)
                    print(body.split('\n', 1)[0])
        return
    depth = 0
    if args and args[0] == '-r':
        depth = int(args[1]); args = args[2:]
    printed = set()
    def show(name, d):
        k = key(funcs, name)
        if not k or funcs[k] in printed:
            return
        body = funcs[k]
        printed.add(body)
        print(body)
        if d > 0:
            for callee in dict.fromkeys(re.findall(r'\bFUN_[0-9a-f]{8}\b', body.split('\n', 1)[1])):
                show(callee, d - 1)
    for a in args:
        show(a, depth)

main()
