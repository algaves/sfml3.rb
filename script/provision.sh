#!/usr/bin/env bash
#
# Installs the target-side development headers SFML needs, inside a
# rake-compiler-dock container. Run as: script/provision.sh <rubygems-platform>
#
# On Linux SFML links X11/Xrandr/Xcursor/Xi/udev/GL from the system, and the
# rake-compiler-dock images ship no X11 development files for any target. On
# Windows and macOS nothing is needed: SFML uses OS libraries and frameworks
# that the mingw toolchain and the osxcross SDK already provide, and FreeType
# is built as a port rather than taken from the system.
#
# This is idempotent -- `rake gem:native` may run it more than once against a
# cached container layer.

set -o errexit
set -o pipefail
set -o nounset

TARGET="${1:?usage: provision.sh <rubygems-platform>}"

# Package names are the same across Debian and Alpine here except for GL and
# udev, which are handled per-branch below.
X11_DEBIAN="libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev"
X11_ALPINE="libx11-dev libxrandr-dev libxcursor-dev libxi-dev eudev-dev mesa-dev"

CMAKE_VERSION=3.31.6

# SFML and CSFML both require CMake >= 3.22, but the rake-compiler-dock images
# are Ubuntu 20.04, whose cmake is 3.16. Install a current one ahead of it on
# PATH -- /usr/local/bin precedes /usr/bin, so the later rake invocation in the
# same container picks this up without any PATH plumbing.
ensure_cmake() {
    if [ -x /usr/local/bin/cmake ]; then
        return
    fi

    local arch dir
    arch="$(uname -m)"
    dir="cmake-${CMAKE_VERSION}-linux-${arch}"

    curl -fsSL "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/${dir}.tar.gz" \
        | sudo tar xz -C /opt
    sudo ln -sf "/opt/${dir}/bin/cmake" /usr/local/bin/cmake
    sudo ln -sf "/opt/${dir}/bin/ctest" /usr/local/bin/ctest

    cmake --version | head -1
}

# No distro ships musl X11 development files, but Alpine's do exactly that and
# apk.static can unpack them into the musl-cross-make sysroot without Alpine
# itself being involved. Both musl targets differ only in sysroot and arch.
provision_musl() {
    local sysroot="$1" arch="$2"
    local alpine="https://dl-cdn.alpinelinux.org/alpine/v3.20"

    if [ ! -x /tmp/apk.static ]; then
        curl -fsSL -o /tmp/apk.static \
            "$alpine/main/x86_64/apk-tools-static-2.14.4-r1.apk" \
            || { echo "could not download apk-tools-static" >&2; exit 1; }
        tar -xzf /tmp/apk.static -C /tmp sbin/apk.static
        sudo mv /tmp/sbin/apk.static /tmp/apk.static
        chmod +x /tmp/apk.static
    fi

    # shellcheck disable=SC2086  # deliberate word splitting into package args
    sudo /tmp/apk.static \
        -X "$alpine/main" -X "$alpine/community" \
        -U --allow-untrusted --arch "$arch" --root "$sysroot" --initdb \
        add $X11_ALPINE
}

ensure_cmake

case "$TARGET" in
x86_64-linux-gnu)
    # The container is itself amd64, so the host's own dev packages are the
    # target's. Ubuntu 20.04 is old enough that linking against its X11 gives
    # the same forward compatibility manylinux relies on.
    sudo apt-get update -qq
    # shellcheck disable=SC2086  # deliberate word splitting into package args
    sudo apt-get install -y -qq $X11_DEBIAN
    ;;

aarch64-linux-gnu)
    # Ubuntu's amd64 archive carries no arm64 binaries, so the arm64 half has
    # to come from ports.ubuntu.com. Pinning the existing entries to amd64
    # first stops apt from trying to fetch arm64 indexes it cannot serve.
    sudo dpkg --add-architecture arm64
    sudo sed -i 's|^deb \(http\)|deb [arch=amd64] \1|' /etc/apt/sources.list
    sudo tee /etc/apt/sources.list.d/arm64.list >/dev/null <<'EOF'
deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports focal main universe
deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports focal-updates main universe
EOF
    sudo apt-get update -qq
    # shellcheck disable=SC2086  # deliberate word splitting into package args
    sudo apt-get install -y -qq ${X11_DEBIAN// /:arm64 }:arm64
    ;;

x86-linux-gnu)
    # Ubuntu's amd64 archive carries i386 too, so unlike the arm64 branch this
    # needs no extra sources list -- just the foreign architecture enabled.
    sudo dpkg --add-architecture i386
    sudo apt-get update -qq
    # shellcheck disable=SC2086  # deliberate word splitting into package args
    sudo apt-get install -y -qq ${X11_DEBIAN// /:i386 }:i386
    ;;

x86_64-linux-musl)
    provision_musl "/usr/x86_64-unknown-linux-musl" x86_64
    ;;

x86-linux-musl)
    provision_musl "/usr/i686-unknown-linux-musl" x86
    ;;

x64-mingw-ucrt | x86-mingw32 | x86_64-darwin | arm64-darwin)
    # Nothing to do -- see the header comment.
    ;;

*)
    echo "provision.sh: unknown target '$TARGET'" >&2
    exit 1
    ;;
esac
