# JumpStart 4th Grade: Haunted Island — native port

A native port of Knowledge Adventure's *JumpStart 4th Grade: Haunted Island*
(1997) for macOS (Apple Silicon and Intel), Windows and Linux. No emulator:
the game's logic was decompiled from the original Windows executable and
ported to C++, on top of a from-scratch reimplementation of its engine
(graphics, sprites, animation, sound mixer, Smacker video, input) using SDL3.

You need your own copy of the original CD. The game reads the disc's data
files directly (from an `.iso` image, a mounted CD, or a folder with the
disc's files); nothing from the disc is included in this repository.

## Installers

All installers are built on a Mac from your copy of the disc, and include
the game data so they work with nothing else needed. They therefore contain
the copyrighted game: keep them for your own use.

| Platform | Installer | Installs to |
| --- | --- | --- |
| macOS 10.15+ (Apple Silicon and Intel) | `JumpStart-4th-Grade-<version>-macOS.dmg`: open it and drag the game to Applications | `/Applications` |
| Windows 10/11 (x64) | `JumpStart-4th-Grade-<version>-Windows-Setup.exe`: setup wizard with Start menu and desktop shortcuts and an uninstaller | `C:\Program Files\JumpStart 4th Grade` |
| Linux x86_64 (glibc 2.34+: Ubuntu 22.04, Debian 12, Fedora 35 or newer) | `JumpStart-4th-Grade-<version>-x86_64.AppImage`: `chmod +x` and run, no install needed | anywhere |
| Debian / Ubuntu | `jumpstart4_<version>_amd64.deb`: `sudo apt install ./jumpstart4_<version>_amd64.deb` | `/usr/bin`, `/usr/share/jumpstart4` |

To build them:

```sh
brew install cmake ninja mingw-w64 makensis lima
# Linux packages are built in Docker; on a Mac, a Lima VM provides it:
limactl create --name=js4docker --vm-type=vz --rosetta template:docker
limactl start js4docker

packaging/make_all.sh [path to .iso or disc folder]   # everything, into dist/
packaging/make_mac.sh | make_windows.sh | make_linux.sh  # or one platform
```

Without an argument the scripts use an `.iso` in the repository folder (or
`game/win/`). The app icon and installer artwork are extracted from the
disc at build time (`packaging/packtool.cpp`, `packaging/art.py`).

The apps are not signed with a paid certificate, so the first launch shows a
warning: on macOS right-click the app and choose **Open** (only needed if the
disk image was downloaded or copied from another Mac); on Windows choose
**More info → Run anyway** in the SmartScreen prompt.

## Building

Requirements: CMake 3.20+, Ninja and a C++17 compiler (Xcode command line
tools on macOS). The first configure downloads and builds SDL3 (statically
linked, so the result is self-contained).

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
open "build/JumpStart 4th Grade.app"          # macOS
./build/jumpstart4 /path/to/game.iso           # Linux
```

On Linux, building SDL needs the usual development packages (X11, Wayland,
ALSA/PulseAudio/PipeWire); `packaging/linux/Dockerfile` lists them. Windows
builds are cross-compiled with MinGW-w64
(`-DCMAKE_TOOLCHAIN_FILE=packaging/windows/mingw-x86_64.cmake`).

## Running

The game looks for its data in this order: a path given on the command line,
data installed with it, the last location used, next to the program, mounted
CDs (`/Volumes` on macOS, drive letters on Windows, `/media` and `/run/media`
on Linux), and otherwise asks you to choose the `.iso` or a folder. The
location is remembered.

Saved players are kept in:

| Platform | Folder |
| --- | --- |
| macOS | `~/Library/Application Support/Knowledge Adventure/JumpStart 4th Grade/` |
| Windows | `%APPDATA%\Knowledge Adventure\JumpStart 4th Grade\` |
| Linux | `~/.local/share/Knowledge Adventure/JumpStart 4th Grade/` |

Keys: full screen with <kbd>⌘F</kbd> on macOS, <kbd>F11</kbd> on Windows and
Linux, or <kbd>Alt</kbd>+<kbd>Enter</kbd> everywhere. Quit with <kbd>⌘Q</kbd>
or by closing the window.

Printing (the game's certificates and reports) uses the system print dialog
on macOS and Windows; on Linux the page is saved as a PNG in the save folder
and opened in your image viewer to print from there.

## How it works

| Directory | Contents |
| --- | --- |
| `src/engine/` | Reimplementation of the Knowledge Adventure engine from 4G.EXE: `.DF` archives and their Huffman/LZ77 decompression, 8-bit graphics with the original blitters and 640×960 back buffer, sprites, `ANG` animations, bitmap fonts and text layout, transitions, the 4-channel sound mixer with lip sync, the Smacker video decoder, event queue and hotspots. `hostfs.cpp` wraps the host file system (UTF-8 paths on Windows too). |
| `src/game/` | The game logic, ported function by function from the decompiled executable. Functions keep their original names (`FUN_00402ec2`) so they can be traced back to the binary. Global state lives in an emulated copy of the executable's data segment (`seg.h`), loaded from the user's `4G.EXE` at startup. |
| `src/platform/` | Printing for each platform. |
| `packaging/` | Installer scripts for macOS, Windows and Linux. |
| `re/` | Reverse-engineering tooling: Ghidra headless scripts (`re/decompile.sh`), the names/prototypes database (`re/names.tsv`), Python tools for the archive format, images and the call graph. |
| `docs/PORTING.md` | Conventions used when porting decompiled code. |

To regenerate the decompilation you need Ghidra 12 and JDK 21; extract the
disc to `game/win/` first.

## Development

Fast development builds on macOS can link Homebrew's SDL3 instead of
building it:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DJS4_SYSTEM_SDL=ON
cmake --build build
```

Debug hooks (environment variables):

| Variable | Effect |
| --- | --- |
| `JS4_SCRIPT=file` | Replays scripted input: `wait ms`, `click x y`, `rclick x y`, `move x y`, `type text`, `char code`, `key vk`, `snap name`, `fuzz count interval`, `quit`, `exit`. |
| `JS4_SNAP_DIR=dir` | Where `snap` writes PNG screenshots of the 640×480 display. |
| `JS4_JUMP=n` | After signing in, enter game module `n` instead of the island. |
| `JS4_SAVE_DIR=dir` | Use a different save directory (e.g. a fresh one for tests). |
| `JS4_AUDIO_DUMP=file` | Write the mixed audio output (raw 8-bit mono 22050 Hz). |

`tools/modtest.sh n` and `tools/fuzz.sh n` run a module headlessly (with
`SDL_AUDIO_DRIVER=dummy`); `tools/fuzz.sh` expects an AddressSanitizer build in
`build-asan`.
