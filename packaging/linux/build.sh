#!/bin/sh
# Runs inside the packaging/linux container (see make_linux.sh):
#   /src   the repository (read-only)
#   /data  the game data: Game.iso or a Disc folder (read-only)
#   /art   artwork from make_art (read-only)
#   /build CMake build directory (cached volume)
# Writes the AppImage and the .deb as a tar stream to stdout.
set -eu
VERSION="$1"
exec 3>&1 1>&2   # keep stdout for the tar stream

cmake -S /src -B /build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build /build --target js4

OUT=/tmp/out
rm -rf "$OUT"
mkdir -p "$OUT"

# Shared install tree: binary in usr/bin, data in usr/share/jumpstart4 (the
# game looks for ../share/jumpstart4 next to its executable).
install_tree() {
    root="$1"
    install -Dm755 /build/jumpstart4 "$root/usr/bin/jumpstart4"
    mkdir -p "$root/usr/share/jumpstart4"
    if [ -d /data/Disc ]; then
        cp -r /data/Disc "$root/usr/share/jumpstart4/Disc"
    else
        install -Dm644 /data/Game.iso "$root/usr/share/jumpstart4/Game.iso"
    fi
    install -Dm644 /src/packaging/linux/jumpstart4.desktop "$root/usr/share/applications/jumpstart4.desktop"
    install -Dm644 /art/icon-256.png "$root/usr/share/icons/hicolor/256x256/apps/jumpstart4.png"
    install -Dm644 /art/icon-512.png "$root/usr/share/icons/hicolor/512x512/apps/jumpstart4.png"
}

echo "== AppImage"
APPDIR=/tmp/AppDir
rm -rf "$APPDIR"
install_tree "$APPDIR"
cp /src/packaging/linux/jumpstart4.desktop "$APPDIR/"
cp /art/icon-256.png "$APPDIR/jumpstart4.png"
cp /art/icon-256.png "$APPDIR/.DirIcon"
ln -s usr/bin/jumpstart4 "$APPDIR/AppRun"
ARCH=x86_64 appimagetool --no-appstream --runtime-file /usr/local/share/runtime-x86_64 \
    "$APPDIR" "$OUT/JumpStart-4th-Grade-$VERSION-x86_64.AppImage"

echo "== .deb"
DEB=/tmp/deb
rm -rf "$DEB"
install_tree "$DEB"
SIZE=$(du -sk "$DEB/usr" | cut -f1)
GLIBC=$(objdump -T /build/jumpstart4 | sed -n 's/.*GLIBC_\([0-9.]*\).*/\1/p' | sort -t. -k1,1n -k2,2n | tail -1)
mkdir -p "$DEB/DEBIAN"
cat > "$DEB/DEBIAN/control" <<EOF
Package: jumpstart4
Version: $VERSION
Architecture: amd64
Section: games
Priority: optional
Installed-Size: $SIZE
Depends: libc6 (>= $GLIBC)
Recommends: libasound2 | libpulse0 | libpipewire-0.3-0, libx11-6 | libwayland-client0, libgl1 | libegl1
Maintainer: Maiko Pruett <66353633+maikopruett@users.noreply.github.com>
Description: JumpStart 4th Grade: Haunted Island (native port)
 Knowledge Adventure's 1997 educational adventure, running natively.
 The game logic was ported from the original Windows executable onto a
 reimplementation of its engine. Includes the game data.
EOF
dpkg-deb --root-owner-group -Zgzip --build "$DEB" "$OUT/jumpstart4_${VERSION}_amd64.deb"

tar -C "$OUT" -cf - . >&3
