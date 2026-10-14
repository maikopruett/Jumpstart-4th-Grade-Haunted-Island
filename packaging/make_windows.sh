#!/bin/sh
# Windows (x64): cross-compiles with MinGW-w64 and builds a Setup.exe with
# NSIS, with the game data included. Needs: brew install mingw-w64 makensis
#
#   packaging/make_windows.sh [path to .iso or disc folder]
set -eu
. "$(dirname "$0")/common.sh"
find_data "${1:-}"
[ -d "$ART" ] || make_art

echo "== building JumpStart4.exe"
WIN_BUILD="$PKG/win"
cmake -S "$ROOT" -B "$WIN_BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="$ROOT/packaging/windows/mingw-x86_64.cmake" \
    -DJS4_WIN_ICON="$ART/AppIcon.ico"
cmake --build "$WIN_BUILD" --target js4

echo "== building the installer"
OUT="$DIST/JumpStart-4th-Grade-$VERSION-Windows-Setup.exe"
if [ -d "$DATA" ]; then DATA_DEF="-DDATA_DIR=$DATA"; else DATA_DEF="-DDATA_ISO=$DATA"; fi
makensis -V2 -INPUTCHARSET UTF8 \
    "-DVERSION=$VERSION" "-DOUTFILE=$OUT" "-DEXE=$WIN_BUILD/JumpStart4.exe" \
    "-DICON=$ART/AppIcon.ico" "-DWELCOME_BMP=$ART/nsis-welcome.bmp" "-DHEADER_BMP=$ART/nsis-header.bmp" \
    "$DATA_DEF" "$ROOT/packaging/windows/installer.nsi"
echo "created $OUT"
