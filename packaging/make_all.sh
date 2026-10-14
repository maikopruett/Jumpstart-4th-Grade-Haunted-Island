#!/bin/sh
# Builds every installer into dist/: macOS .dmg, Windows Setup.exe, Linux
# AppImage and .deb.
#
#   packaging/make_all.sh [path to .iso or disc folder]
set -eu
DIR=$(dirname "$0")
. "$DIR/common.sh"
find_data "${1:-}"
make_art
sh "$DIR/make_mac.sh" "$DATA"
sh "$DIR/make_windows.sh" "$DATA"
sh "$DIR/make_linux.sh" "$DATA"
ls -la "$DIST"
