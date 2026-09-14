# sfml3.rb

Ruby bindings for [SFML 3](https://www.sfml-dev.org/), via its C API, [CSFML](https://github.com/SFML/CSFML).

## Status

Bound against **CSFML 3**. See [TODO.md](TODO.md) for which parts of the SFML 3 API are ported so
far, and [CHANGELOG.md](CHANGELOG.md) for what changed recently.

## Install

```sh
gem install sfml3-rb
```

On a platform with a precompiled gem this installs a binary with FreeType, SFML 3 and CSFML 3
already linked in — no toolchain, no build, nothing to install system-wide:

| Platform | Ruby | Status |
| --- | --- | --- |
| `x86_64-linux-gnu` | 3.1 – 4.0 | built, installed and tested |
| `x86-linux-gnu` | 3.1 – 4.0 | 32-bit glibc Linux |
| `x86_64-linux-musl` | 3.1 – 4.0 | Alpine and other musl systems |
| `x86-linux-musl` | 3.1 – 4.0 | 32-bit musl |
| `x64-mingw-ucrt` | 3.1 – 4.0 | 64-bit Windows, RubyInstaller 3.1+ |
| `x86-mingw32` | 3.1 – **3.4** | 32-bit Windows |
| `aarch64-linux-gnu`, `x86_64-darwin`, `arm64-darwin` | 3.1 – 4.0 | experimental, not yet built |

Each gem carries one extension per Ruby ABI. Two gaps come from upstream rather than from this
project: **RubyInstaller publishes no 32-bit Ruby 4.0**, so `x86-mingw32` stops at 3.4; and 64-bit
Windows before Ruby 3.1 used a different platform (`x64-mingw32`), which is not built.

Anywhere else — macOS, ARM, the BSDs — RubyGems falls back to the source gem, which downloads and
builds FreeType, SFML 3 and CSFML 3 from pinned, checksum-verified tarballs at install time. That
takes a few minutes and needs:

* Ruby >= 3.1
* A C/C++ toolchain and CMake >= 3.22
* On Linux, the X11/udev/OpenGL development headers SFML links against — these can't be bundled.
  On Fedora:

  ```sh
  sudo dnf install cmake gcc-c++ libX11-devel \
    libXrandr-devel libXcursor-devel libXi-devel systemd-devel libglvnd-devel
  ```

  or on Debian/Ubuntu:

  ```sh
  sudo apt-get install cmake build-essential libx11-dev \
    libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev
  ```

Each installed gem version builds its own copy; there's no build cache shared across versions.

To link against a system CSFML 3 instead (no download, no build):

```sh
gem install sfml3-rb -- --enable-system-libraries
```

## Usage

```ruby
require 'sfml'
include SFML

window = Window.new VideoMode.new(640, 480, 32), 'SFML'
event = Event.new

while window.is_open?
  while window.poll_event! event
    window.close! if event.type == 'closed'
  end

  window.clear [51, 76, 102, 255] # [r, g, b, a], 0-255
  window.display
end
```

See [`test/hello-world.rb`](test/hello-world.rb) for a fuller example with shapes and transforms.

## Development

```sh
rake ports    # build the vendored FreeType + SFML 3 + CSFML 3 (rake compile does this too)
rake compile  # build the C extension into lib/sfml/
rake test     # compile, then run the test suite
rake gem      # build the source gem into pkg/
```

`rake githooks:install` points your checkout at the committed `.githooks/` pre-commit hook, which
lints staged Ruby with RuboCop and auto-formats staged C with clang-format (`sudo dnf install
clang-tools-extra` on Fedora for the C side).

`CMakeLists.txt` is a CLion/IDE convenience build against the same vendored `ports/` prefix — `rake`
is the build of record.

### Building the binary gems

Cross-compilation runs in [rake-compiler-dock](https://github.com/rake-compiler/rake-compiler-dock),
which supplies the cross toolchains and the cross-compiled rubies. It needs Docker or Podman, and
pulls a large image per platform on first use.

```sh
rake platforms            # list the targets
rake gem:x86_64-linux-gnu # build one platform into pkg/
rake gem:native           # build all of them
```

Each run provisions the container with [`script/provision.sh`](script/provision.sh) — a current
CMake everywhere, plus the target-side X11/udev/GL development files on Linux targets, which the
images don't ship. Windows and macOS need nothing extra: SFML uses OS libraries and frameworks the
mingw toolchain and the osxcross SDK already provide.

Binary gems carry one extension per Ruby ABI under `lib/sfml/<major.minor>/`; `lib/sfml.rb` prefers
that and falls back to the single `lib/sfml/sfml_ext.so` a source build installs.

## License

[0BSD](LICENSE.md)

---

Developed by: [Algaves](https://github.com/algaves/sfml3.rb) @ 2026
