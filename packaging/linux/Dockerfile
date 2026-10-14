# Build environment for the Linux packages (x86_64). Ubuntu 22.04 keeps the
# glibc requirement low (2.34) so the binary runs on most current distros.
FROM --platform=linux/amd64 ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake ninja-build git pkg-config ca-certificates curl file dpkg-dev \
        libasound2-dev libpulse-dev libpipewire-0.3-dev libdbus-1-dev libudev-dev libibus-1.0-dev \
        libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev \
        libxtst-dev libxkbcommon-dev libdrm-dev libgbm-dev libgl1-mesa-dev libegl1-mesa-dev \
        libgles2-mesa-dev libwayland-dev libwayland-bin wayland-protocols libdecor-0-dev \
    && rm -rf /var/lib/apt/lists/*

# AppImage tooling (run extracted: there is no FUSE inside containers).
RUN curl -fsSL -o /usr/local/bin/appimagetool \
        https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage \
    && chmod +x /usr/local/bin/appimagetool \
    # Clear the AppImage magic in the ELF padding so binfmt handlers (e.g.
    # Rosetta or qemu on non-x86 hosts) accept it as a plain ELF binary.
    && printf '\0\0\0' | dd of=/usr/local/bin/appimagetool bs=1 seek=8 count=3 conv=notrunc \
    && curl -fsSL -o /usr/local/share/runtime-x86_64 \
        https://github.com/AppImage/type2-runtime/releases/download/continuous/runtime-x86_64
ENV APPIMAGE_EXTRACT_AND_RUN=1
