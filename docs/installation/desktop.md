---
layout: default
title: Desktop (macOS, Linux, Windows)
parent: Installation
nav_order: 1
---

# Desktop installation

## Precompiled gems

| Platform | Ruby | Status |
| --- | --- | --- |
| `x86_64-linux-gnu` | 3.1 – 4.0 | built, installed and tested |
| `x86-linux-gnu` | 3.1 – 4.0 | 32-bit glibc Linux |
| `x86_64-linux-musl` | 3.1 – 4.0 | Alpine and other musl systems |
| `x86-linux-musl` | 3.1 – 4.0 | 32-bit musl |
| `x64-mingw-ucrt` | 3.1 – 4.0 | 64-bit Windows, RubyInstaller 3.1+ |
| `x86-mingw32` | 3.1 – **3.4** | 32-bit Windows |
| `aarch64-linux-gnu` | 3.1 – 4.0 | experimental |
| `aarch64-linux-musl`, `arm-linux-gnu`, `arm-linux-musl` | 3.1 – 4.0 | experimental, not yet built |
| `aarch64-mingw-ucrt` | 3.4 – **4.0** | experimental, 64-bit Windows on ARM |
| `x86_64-darwin`, `arm64-darwin` | 3.1 – 4.0 | experimental, not yet built |

## Building from source

Anywhere else (macOS, ARM, BSDs), RubyGems falls back to the source gem. Requirements: Ruby >= 3.1, a C/C++ toolchain, CMake >= 3.22, and on Linux the X11/udev/OpenGL development headers.

Fedora:
```sh
sudo dnf install cmake gcc-c++ libX11-devel libXrandr-devel libXcursor-devel libXi-devel systemd-devel libglvnd-devel
```

Debian/Ubuntu:
```sh
sudo apt-get install cmake build-essential libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev
```

Linking against system libraries:
```sh
gem install sfml3-rb -- --enable-system-libraries
```
