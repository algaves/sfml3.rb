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
  and the system layer, with every `sf*` entry point tracked in [ROADMAP.md](ROADMAP.md).
* **A Rubyesque (Matz-like) layer** over that raw binding, in
  [`lib/sfml/rubyesque.rb`](lib/sfml/rubyesque.rb): `?` predicates, `!` mutators, block iterators
  (`poll_events!`, `render!`, `open!`), scoped resources (`WindowBase.open`,
  `SoundBufferRecorder.record!`, `Clock.measure`), positional `Class[...]` constructors
  (`VideoMode[...]`, `Vector2[...]`, `CircleShape[...]`, ...) and the `Style`/`State` flag
  namespaces. The pre-Rubyesque names remain as deprecated aliases, so nothing breaks. See
  [below](#rubyesque-matz-like-layer).
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
* [Rubyesque (Matz-like) layer](#rubyesque-matz-like-layer)
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
include SF::Window
include SF::Graphics
include SF::System
include SF::Audio
include SF::Network

window = Window.new VideoMode[640, 480, 32], 'SFML'

while window.open?
  window.poll_events! do |event|
    window.close! if event.closed?
  end

  window.clear! [51, 76, 102, 255] # [r, g, b, a], 0-255
  window.display!
end
```

The primary spellings (`open?`, `clear!`, `display!`, ...) are pure-Ruby Rubyesque (Matz-like)
over the native API. The names that shipped before them (`is_open?`, `clear`, `display`, ...) still
work but warn that they are deprecated.

## Rubyesque (Matz-like) layer

`lib/sfml/rubyesque.rb` layers the Rubyesque (Matz-like) API on top of the raw CSFML binding:
predicate methods end in `?`, methods that change state end in `!`, and block-scoped helpers handle
setup and teardown. Nothing is lost — the native surface is still there, unchanged.

### Windows: scoped setup, block events, scoped frames

```ruby
# The window is closed automatically when the block returns or raises.
RenderWindow.open(VideoMode.new(640, 480, 32), 'Game Title') do |window|
  window.poll_events! do |event|          # yields every pending event
    window.close! if event.closed?
    puts "Key pressed: #{event.code}" if event.key_pressed?
  end

  window.render!(clear_color: Color::BLACK) do |target|
    target.draw sprite
    target.draw text
  end
end
```

`poll_events!` returns an `Enumerator` without a block; `render!` clears, yields the window,
then presents. `window.open?`, `window.focused?` and `window.visible?` are the predicates;
`request_focus!` asks the window manager for focus, and `close!` closes the window. The scoped
constructor lives on `WindowBase`, so `WindowBase.open`, `Window.open` and `RenderWindow.open`
all behave the same way and return the open window when called without a block.

A window also takes a block-oriented loop. `window.open!` yields it repeatedly while it is open
and closes it afterwards, `window.poll_event! { |event| ... }` drains the pending events (the
block form of `poll_events!`), and `window.wait_event! { |event| ... }` blocks for one. Without a
block, `poll_event!`/`wait_event!` keep their native `poll_event!(event) -> bool` form.

```ruby
window = Window.new(VideoMode[640, 480, 32], 'Game Title', Style::DEFAULT)

window.open! do
  window.poll_event! do |event|
    window.close! if event.closed? || (event.key_pressed? && event.code == :escape)
  end

  window.render! do |target|
    target.draw sprite
  end
end
```

### Construction: `VideoMode`, `Style`/`State` and `Class[...]`

```ruby
window = RenderWindow.new(
  VideoMode[640, 480, 32],   # VideoMode[width, height, bits = 32]
  'Hello world!',
  Style::DEFAULT,            # or Style::TITLEBAR | Style::RESIZE
  State::WINDOWED            # or State::FULLSCREEN
)
```

`Style` and `State` are Integer flag namespaces mirroring CSFML (`sfStyle`/`sfWindowState`), so
they combine with `|` and slot into the constructor where the `:default` / `:windowed` symbols did.

The value and resource classes also take a positional `Class[...]` constructor; every `.new` form
keeps working.

```ruby
Vector2[x, y]                  # also Vector2[[x, y]]
Vector3[x, y, z]
Color[r, g, b]                 # Color[r, g, b, a] or Color[packed]
Rect[left, top, width, height]
Time[seconds]                  # always seconds
View[Rect[left, top, width, height]]   # or View[center, size]
Text[font, 'Hello', 24]
Vertex[Vector2[x, y], Color[r, g, b]]
Texture[[width, height]]       # Texture / Image / RenderTexture take a size
Sprite[texture]

CircleShape[radius, [x, y]]            # position defaults to [0, 0]
RectangleShape[x, y, width, height]
ConvexShape[[x0, y0], [x1, y1], ...]   # points as arrays or Vector2s
```

### Audio: predicates, banged playback, scoped recording

```ruby
music.play! if music.stopped?
music.pause! if music.playing? && pause_condition

if sound.playing? || sound.paused?
  sound.stop!
end

# Records until the block returns and returns the resulting SoundBuffer.
buffer = SoundBufferRecorder.record!(sample_rate: 44_100, device: nil) do |recorder|
  # capture happens while the block runs
end
Sound.new(buffer)
```

`SoundSource#playing?`, `#paused?` and `#stopped?` are shared by `Sound`, `SoundStream` and
`Music`; `play!`, `pause!` and `stop!` are their mutating counterparts.

### Sensors and input devices

```ruby
if Sensor.available?(:gyroscope)
  Sensor.enable!(:gyroscope)
  rotation = Sensor.value(:gyroscope)
  Sensor.disable!(:gyroscope)
end

Joystick.connected?(0)                # => true/false
Joystick.button_count(0)
Joystick.button_pressed?(0, 0)
Joystick.axis?(0, :z)

Keyboard.key_pressed?(:space)
Touch.down?(0)
Touch.position(0, relative_to: window)
```

### Clipboard

```ruby
Clipboard.content = 'Copy this text'
puts Clipboard.content if Clipboard.has_text?
Clipboard.clear!
```

### System: clocks and sleep

```ruby
elapsed = Clock.measure do
  SF::System::Sleep.sleep!(Time.seconds(0.5))
end

puts "Executed in #{elapsed.as_seconds}s"
```

`clock.restart!` and `clock.running?` are on every `Clock`.

### Events

Event types are strings (`'closed'`, `'resized'`, `'key-pressed'`, ...) and every payload is a
Hash (`event.key[:code]`, `event.mouse_button[:button]`, ...), as before. On top of that every
kind has a predicate — `event.closed?`, `event.key_pressed?`, `event.mouse_moved?`,
`event.touch_began?`, ... — and `event.code` is the `event.key[:code]` shortcut.

### Name changes at a glance

Nothing is removed: the pre-Rubyesque spelling keeps working and warns once with the line that
called it. The primary names are the right-hand column.

| Before (deprecated)             | Now (primary)                                 |
| ------------------------------- | --------------------------------------------- |
| `window.is_open?`               | `window.open?`                                |
| `window.focus?`                 | `window.focused?`                             |
| `window.request_focus`          | `window.request_focus!`                       |
| `window.clear`                  | `window.clear!`                               |
| `window.display`                | `window.display!`                             |
| `window.poll_event!(event)`     | `window.poll_events! { \|event\| }`           |
| `sound.play` / `pause` / `stop` | `sound.play!` / `pause!` / `stop!`            |
| `sound.status == :playing`      | `sound.playing?` (also `paused?`, `stopped?`) |
| `Keyboard.pressed?`             | `Keyboard.key_pressed?`                       |
| `Joystick.has_axis?`            | `Joystick.axis?`                              |
| `Clipboard.string` / `string=`  | `Clipboard.content` / `content=`              |
| `SF.sleep`                    | `SF.sleep!` (or `SF::System::Sleep.sleep!`)       |

The Rubyesque layer also fills gaps the native surface leaves: `WindowBase.open` and `window.render!`,
`SoundBufferRecorder.record!`, `Clock.measure`, `Sensor.enable!`/`disable!`,
`Clipboard.has_text?`/`clear!`, `Touch.position(finger, relative_to: window)` and every
`event.*?` predicate are new, with no pre-Rubyesque equivalent.

See [`examples/hello_shapes.rb`](examples/hello_shapes.rb) for a minimal walkthrough of the five
building blocks (window, events, transformables, drawables, primitive shapes), and
[`examples/bouncing_shapes.rb`](examples/bouncing_shapes.rb) for an interactive take on the same
components. [`examples/rubyesque/`](examples/rubyesque) is one small teaching script per slice of
the Rubyesque layer -- window, events, audio, input, system and the deprecation path -- with the
same cheat sheet as above. [`examples/subsystems/`](examples/subsystems) has one demo per module (Listener, GLSL,
Clipboard, Joystick, Keyboard, Mouse, Sensor, Touch, Vulkan, window styles, DNS, audio devices, and
sprites/textures/images), and [`examples/games/`](examples/games) has eleven playable games --
Snake, Breakout, Asteroids, Platformer, Tron, Flappy Bird, Doodle Jump, Xonix, Tetris, Racing and
Chess -- plus a small GUI toolkit (`examples/menu.rb`) and a control-panel demo. [`examples/README.md`](examples/README.md)
documents them all. They need a display (`xvfb-run -a` headlessly), run without interacting when
`SFML_EXAMPLE_FRAMES` is set, and are not part of the test suite.

## Documentation

* [API reference](https://algaves.github.io/sfml3.rb/) — every class, module, method and constant,
  built from the YARD comments in `ext/**/*.c` and deployed to GitHub Pages by CI. The same docs
  are also generated on [RubyDoc.info](https://rubydoc.info/gems/sfml3-rb).
* RBS type signatures (`sig/**/*.rbs`) describe the whole API, including the native classes, for
  RBS-aware editors. `rake rbs` validates the signatures and `rake steep` type-checks `lib/`
  against them.
* [CHANGELOG.md](CHANGELOG.md) — what changed in each release.
* [ROADMAP.md](ROADMAP.md) — module-by-module porting coverage, what is deliberately unbound, and
  the SFML 3.1.0 elements still pending upstream.

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
RenderTexture). `SF::Graphics::RenderWindow < SF::Window::Window < SF::Window::WindowBase` and includes the
`SF::Graphics::RenderTarget` module, so a drawable's `#draw` accepts a `RenderWindow` or a
`RenderTexture` directly; `SF::Graphics::Target` remains as the legacy generic wrapper.

The class hierarchy mirrors SFML's own. `SF::Graphics::Drawable` is a mixin included by every drawable;
`SF::Graphics::Transformable` is a mixin included by `Sprite`, `Text` and `Shape`, whose position/rotation/
scale/origin surface is implemented once and dispatched to each class's CSFML entry point. `Shape`
is the base of `CircleShape` (also available as `Circle`), `RectangleShape` and `ConvexShape`. On
the audio side `SF::Audio::SoundSource` is the base of `Sound`, `SoundStream` and `Music`, and
`SF::Audio::Music < SF::Audio::SoundStream`, with `SF::Audio::SoundBufferRecorder < SF::Audio::SoundRecorder`.

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
