#!/bin/sh
# Syntax-check individual source files without building the whole project.
cd "$(dirname "$0")/.."
for f in "$@"; do
  clang++ -std=c++17 -fsyntax-only -Wall -Wno-unused-variable -Wno-unused-but-set-variable \
    -Wno-unused-function -Wno-deprecated-declarations -Isrc -I/opt/homebrew/opt/sdl3/include "$f" || exit 1
done
echo "ok"
