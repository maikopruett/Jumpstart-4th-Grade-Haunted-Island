#!/bin/sh
# macOS: universal (Apple Silicon + Intel) app in a drag-to-Applications
# disk image, with the game data inside the app.
#
#   packaging/make_mac.sh [path to .iso or disc folder]
set -eu
. "$(dirname "$0")/common.sh"
find_data "${1:-}"
[ -d "$ART" ] || make_art
python_env

echo "== building the universal app"
configure_mac
cmake --build "$HOST_BUILD" --target js4

echo "== assembling the app"
APP_NAME="JumpStart 4th Grade"
STAGE="$PKG/mac-stage"
STAGED="$STAGE/$APP_NAME.app"
rm -rf "$STAGE"
mkdir -p "$STAGE"
ditto "$HOST_BUILD/$APP_NAME.app" "$STAGED"
mkdir -p "$STAGED/Contents/Resources"
cp "$ART/AppIcon.icns" "$STAGED/Contents/Resources/"
if [ -d "$DATA" ]; then
    ditto "$DATA" "$STAGED/Contents/Resources/Disc"
else
    cp "$DATA" "$STAGED/Contents/Resources/Game.iso"
fi
codesign --force --deep --sign - "$STAGED"

echo "== creating the disk image"
DMG="$DIST/JumpStart-4th-Grade-$VERSION-macOS.dmg"
rm -f "$DMG"
"$PKG/venv/bin/dmgbuild" -s "$ROOT/packaging/mac/dmg_settings.py" \
    -D app="$STAGED" -D icon="$ART/AppIcon.icns" -D background="$ART/background.png" \
    "$APP_NAME" "$DMG"
rm -rf "$STAGE"
echo "created $DMG"
