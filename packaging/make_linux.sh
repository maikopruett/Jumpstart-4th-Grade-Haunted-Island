#!/bin/sh
# Linux (x86_64): AppImage and .deb, built in an Ubuntu 22.04 container, with
# the game data included. Needs Docker; on a Mac a Lima VM works:
#   limactl create --name=js4docker --vm-type=vz --rosetta template:docker
#   limactl start js4docker
#
#   packaging/make_linux.sh [path to .iso or disc folder]
set -eu
. "$(dirname "$0")/common.sh"
find_data "${1:-}"
[ -d "$ART" ] || make_art

if [ -z "${DOCKER_HOST:-}" ] && command -v limactl >/dev/null && limactl list -q 2>/dev/null | grep -qx js4docker; then
    DOCKER_HOST=$(limactl list js4docker --format 'unix://{{.Dir}}/sock/docker.sock')
    export DOCKER_HOST
fi

echo "== preparing the build container"
docker build --platform linux/amd64 -t js4-linux-build "$ROOT/packaging/linux"

if [ -d "$DATA" ]; then DATA_MOUNT="$DATA:/data/Disc:ro"; else DATA_MOUNT="$DATA:/data/Game.iso:ro"; fi
echo "== building in the container"
docker run --rm --platform linux/amd64 \
    -v "$ROOT:/src:ro" -v "$DATA_MOUNT" -v "$ART:/art:ro" -v js4-linux-build:/build \
    js4-linux-build sh /src/packaging/linux/build.sh "$VERSION" | { tar -C "$DIST" -xf -; cat >/dev/null; }
ls -la "$DIST"/*.AppImage "$DIST"/*.deb
