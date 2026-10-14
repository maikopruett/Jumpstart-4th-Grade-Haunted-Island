# Shared setup for the packaging scripts (sourced, not run).
#
# Sets ROOT, VERSION, DATA (the .iso or disc folder to bundle), DIST (output
# folder) and ART (artwork extracted from the game data), building the macOS
# host tools first if needed.

ROOT=$(cd "$(dirname "$0")/.." && pwd)
VERSION=$(sed -n 's/^set(APP_VERSION "\(.*\)")/\1/p' "$ROOT/CMakeLists.txt")
DIST="$ROOT/dist"
PKG="$ROOT/build-pkg"
ART="$PKG/art"
HOST_BUILD="$PKG/mac"
mkdir -p "$DIST" "$PKG"

# Game data: first argument, else an .iso or game/win in the repository.
find_data() {
    DATA="${1:-}"
    if [ -z "$DATA" ]; then
        for cand in "$ROOT"/*.iso "$ROOT"/*.ISO "$ROOT/game/win"; do
            if [ -e "$cand" ]; then DATA="$cand"; break; fi
        done
    fi
    if [ -z "$DATA" ] || [ ! -e "$DATA" ]; then
        echo "$(basename "$0"): can't find the game data; pass the .iso or disc folder" >&2
        exit 1
    fi
    echo "game data: $DATA"
}

# The macOS build doubles as the host build for packtool.
configure_mac() {
    if [ ! -f "$HOST_BUILD/build.ninja" ]; then
        cmake -S "$ROOT" -B "$HOST_BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Release \
            "-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64"
    fi
}

python_env() {
    if [ ! -x "$PKG/venv/bin/dmgbuild" ]; then
        python3 -m venv "$PKG/venv"
        "$PKG/venv/bin/pip" install -q dmgbuild pillow
    fi
    PY="$PKG/venv/bin/python3"
}

# Icon and artwork, extracted from the user's own disc.
make_art() {
    configure_mac
    cmake --build "$HOST_BUILD" --target packtool
    python_env
    rm -rf "$ART"
    mkdir -p "$ART"
    "$HOST_BUILD/packtool" "$DATA" icon "$ART/icon32.png"
    "$HOST_BUILD/packtool" "$DATA" image 0x1f4 0x1f5 "$ART/tombstone.png"
    "$PY" "$ROOT/packaging/art.py" "$ART/icon32.png" "$ART/tombstone.png" "$ART"
    iconutil -c icns "$ART/AppIcon.iconset" -o "$ART/AppIcon.icns"
}
