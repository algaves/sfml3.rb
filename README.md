# sfml3.rb

Ruby bindings for [SFML 3](https://www.sfml-dev.org/), via its C API, [CSFML](https://github.com/SFML/CSFML).

[![Ruby](https://img.shields.io/badge/ruby-3.1%2B-red?style=flat-square)](https://www.ruby-lang.org/)
[![Test](https://img.shields.io/github/actions/workflow/status/algaves/sfml3.rb/test.yaml?style=flat-square)](https://img.shields.io/github/actions/workflow/status/algaves/sfml3.rb/test.yaml?style=flat-square)
[![Check](https://img.shields.io/github/actions/workflow/status/algaves/sfml3.rb/check.yaml?style=flat-square)](https://img.shields.io/github/actions/workflow/status/algaves/sfml3.rb/check.yaml?style=flat-square)
[![Docs](https://img.shields.io/badge/docs-GitHub%20Pages-4c9a2a?style=flat-square)](https://algaves.github.io/sfml3.rb/)
[![Gem Version](https://img.shields.io/gem/v/sfml3-rb?style=flat-square)](https://rubygems.org/gems/sfml3-rb)
[![Gem Downloads](https://img.shields.io/gem/dt/sfml3-rb?style=flat-square)](https://rubygems.org/gems/sfml3-rb)
[![License](https://img.shields.io/badge/license-0BSD-green?style=flat-square)](LICENSE.md)

Latest release: **0.3.1**, bound against **CSFML 3**.

## Features

* **Broad coverage of SFML 3**, bound through CSFML: windows and events, graphics, audio, network,
  and the system layer, with every `sf*` entry point tracked in [TODO.md](TODO.md).
* **Precompiled binary gems** for common platforms, with FreeType, SFML 3 and CSFML 3 statically
  linked in — no toolchain and nothing to install system-wide.
* **A source fallback everywhere else**, which downloads and builds the pinned, checksum-verified
  dependencies at install time, so the gem works on macOS, ARM and the BSDs out of the box.
* **Complete API documentation and types**: every class, module, method and constant is documented
  on the [docs site](https://algaves.github.io/sfml3.rb/) and covered by RBS signatures shipped in
  the gem.
* **A close fit to SFML's own model**: classes mirror the C++ types (WindowBase, Window,
  RenderWindow, Texture, Sprite, Sound, ...) minus the parts that only exist in C++, like
  `std::string` and exceptions.

## Table of Contents

* [Installation](#installation)
* [Quick Start](#quick-start)
* [Documentation](#documentation)
* [Development](#development)
* [Contributing](#contributing)
* [Acknowledgements](#acknowledgements)
* [License](#license)

## Installation

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
| `aarch64-linux-gnu` | 3.1 – 4.0 | experimental, cross-built and run on a native arm64 CI runner |
| `aarch64-linux-musl`, `arm-linux-gnu`, `arm-linux-musl` | 3.1 – 4.0 | experimental, not yet built |
| `aarch64-mingw-ucrt` | 3.4 – **4.0** | experimental, 64-bit Windows on ARM |
| `x86_64-darwin`, `arm64-darwin` | 3.1 – 4.0 | experimental, not yet built |

Each gem carries one extension per Ruby ABI. Three gaps come from upstream rather than from this
project: **RubyInstaller publishes no 32-bit Ruby 4.0**, so `x86-mingw32` stops at 3.4; 64-bit
Windows before Ruby 3.1 used a different platform (`x64-mingw32`), which is not built; and the
cross-compilation image for `aarch64-mingw-ucrt` carries no cross Ruby older than 3.4, so that
gem has no floor below it.

### Building from source

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

On Debian/Ubuntu:

```sh
sudo apt-get install cmake build-essential libx11-dev \
  libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev
```

Each installed gem version builds its own copy; there's no build cache shared across versions.

### Linking against system libraries

To link against a system CSFML 3 instead (no download, no build):

```sh
gem install sfml3-rb -- --enable-system-libraries
```

## Quick Start

```ruby
require 'sfml'
include SFML

window = Window.new VideoMode.new(640, 480, 32), 'SFML'
event  = Event.new

while window.is_open?
  while window.poll_event! event
    window.close! if event.type == 'closed'
  end

  window.clear [51, 76, 102, 255] # [r, g, b, a], 0-255
  window.display
end
```

Event types are strings (`'closed'`, `'resized'`, `'key-pressed'`, ...); keys and buttons are enums.

See [`test/hello-world.rb`](test/hello-world.rb) for a fuller example with shapes and transforms,
and [`test/matrix-transformable.rb`](test/matrix-transformable.rb) for a visual demo. Neither is
part of the test suite.

## Documentation

* [API reference](https://algaves.github.io/sfml3.rb/) — every class, module, method and constant,
  built from the YARD comments in `ext/**/*.c` and deployed to GitHub Pages by CI. The same docs
  are also generated on [RubyDoc.info](https://rubydoc.info/gems/sfml3-rb).
* RBS type signatures (`sig/**/*.rbs`) describe the whole API, including the native classes, for
  RBS-aware editors. `rake rbs` validates the signatures and `rake steep` type-checks `lib/`
  against them.
* [CHANGELOG.md](CHANGELOG.md) — what changed in each release.
* [TODO.md](TODO.md) — module-by-module porting coverage and what is deliberately unbound.

### IDE setup (RubyMine)

`sig/**/*.rbs` is the only machine-readable description of the API: the native extension cannot be
introspected, so an editor either reads the signatures or sees nothing. RBS support lives in
**RubyMine** (and IntelliJ IDEA Ultimate with the Ruby plugin); CLion and other C/C++ IDEs show
`.rbs` files as plain text.

1. Open the project in RubyMine and point **Settings → Languages & Frameworks → Ruby SDK** at the
   interpreter you build against (Ruby 3.1+). Keep the C sources in CLion/clangd.
2. Run `bundle install` so the `rbs` gem (3.2+) is available to that interpreter.
3. RubyMine indexes `sig/` automatically; completion, type info (`Ctrl+Shift+P`), parameter info
   and *Navigate → Type Signature* then work for `Window.new` and the rest of the API.
4. For a full type check, run `steep check` from *Run anything* (`Ctrl` twice); `Steepfile` points
   it at `lib/` and `sig/`.

## Development

```sh
rake ports    # build the vendored FreeType + SFML 3 + CSFML 3 (rake compile does this too)
rake compile  # build the C extension into lib/sfml/
rake test     # compile, then run the test suite
rake gem      # build the source gem into pkg/
rake yard     # build API docs into doc/
rake rbs      # validate sig/**/*.rbs
rake steep    # type-check lib/ against sig/**/*.rbs
```

`rake githooks:install` points your checkout at the committed `.githooks/` pre-commit hook, which
lints staged Ruby with RuboCop and auto-formats staged C with clang-format (`sudo dnf install
clang-tools-extra` on Fedora for the C side).

`CMakeLists.txt` is a CLion/IDE convenience build against the same vendored `ports/` prefix — `rake`
is the build of record.

The C extension under `ext/` mirrors SFML's own subsystems, with each binding's `.c` and `.h`
side by side: `core/` (CSFML umbrella header, macros, exceptions, UTF-32 conversion),
`system/` (Clock, Time, vectors, streams), `window/` (Window, Event, VideoMode, and the input
devices), `graphics/` (shapes, Color, Transform, View, Texture, Text, Shader, the render targets),
`audio/` and `network/`. Includes are subsystem-relative, e.g. `#include "graphics/circle.h"`.

Three `.inc` files hold method bodies shared by several classes and are included once per class
with a different macro prefix: `audio/sound_source.inc` (Sound, Music, SoundStream),
`window/window_base.inc` (WindowBase, Window) and `graphics/render_target.inc` (Window,
RenderTexture). `SFML::RenderWindow < SFML::Window < SFML::WindowBase` and includes the
`SFML::RenderTarget` module, so a drawable's `#draw` accepts a `RenderWindow` or a
`RenderTexture` directly; `SFML::Target` remains as the legacy generic wrapper.

Note that mkmf flattens object files to their basenames, so every `.c` filename has to stay
unique across the whole tree — and that `$srcs` is baked into the generated Makefile, so after
adding a `.c` file run `touch ext/extconf.rb && rake compile` (or `rake clean compile`), otherwise
it is silently left out of the link.

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

## Contributing

Bug reports and pull requests are welcome on
[GitHub](https://github.com/algaves/sfml3.rb/issues). Before opening a pull request:

```sh
bundle install
bundle exec rake test   # build the extension and run the suite
bundle exec rubocop     # lint Ruby (CI enforces this)
```

Install the pre-commit hook with `bundle exec rake githooks:install` so staged Ruby is linted and
staged C is formatted automatically. The C build is warning-clean under `SFML_STRICT=1`; keep it
that way.

## Acknowledgements

This gem would not exist without [SFML](https://www.sfml-dev.org/) and its C binding,
[CSFML](https://github.com/SFML/CSFML), both maintained by the SFML team. The source build also
vendors and links FreeType, Ogg, Vorbis and FLAC; see [`ext/ports.rb`](ext/ports.rb) and the
[LICENSE](LICENSE.md) for their terms.

## License

This project is licensed under the BSD Zero Clause License (0BSD) - see the [LICENSE](LICENSE.md) file for details.

---

Developed by: [Algaves](https://github.com/algaves/sfml3.rb) @ 2026
