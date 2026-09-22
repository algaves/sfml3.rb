# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
* **A pure-Ruby Rubyesque (Matz-like) layer** (`lib/sfml/rubyesque.rb`), loaded by
  `require 'sfml'`.
  Predicate methods end in `?` (`window.open?`, `window.focused?`, `window.visible?`,
  `sound.playing?`, `Keyboard.key_pressed?`, `Joystick.axis?`, `Clipboard.has_text?`),
  state-changing methods end in `!` (`window.close!`, `clear!`, `display!`, `play!`, `pause!`,
  `stop!`, `Sensor.enable!`/`disable!`, `Clipboard.clear!`, `SFML.sleep!`), and block-scoped
  helpers manage resources: `WindowBase.open` (`Window.open`/`RenderWindow.open`),
  `window.poll_events!`, `window.render!`, `SoundBufferRecorder.record!` and `Clock.measure`.
  Event kinds gain predicates (`event.closed?`, `event.key_pressed?`, ...) and `event.code` is
  the `event.key[:code]` shortcut. `SFML::Sleep.sleep!` mirrors the namespace spelling, and
  `Touch.position` accepts `relative_to:`.
* **`SFML::Style` and `SFML::State`**, Integer flag namespaces mirroring the CSFML `sfStyle` and
  `sfWindowState` values, so `Style::DEFAULT` / `State::FULLSCREEN` and `Style::TITLEBAR |
  Style::RESIZE` slot into the window constructors where the `:default`/`:windowed` symbols did.
* **Positional `Class[...]` constructors**: `VideoMode[w, h, bits = 32]`, `Vector2[x, y]`,
  `Vector3[x, y, z]`, `Color[r, g, b]`, `Rect[left, top, width, height]`, `Time[seconds]`,
  `View[rect]`/`View[center, size]`, `Text[font, ...]`, `Vertex[...]`, `Sprite[texture]`,
  `Texture[size]`, `Image[size]`, `RenderTexture[size]`, `CircleShape[r, [x, y]]`,
  `RectangleShape[x, y, w, h]` and `ConvexShape[[x0, y0], ...]`. Every `.new` form is unchanged.
* **`Window#open!` and block forms of `poll_event!`/`wait_event!`**: `open!` runs a block while the
  window is open and closes it, `poll_event! { |event| ... }` drains the pending events, and
  `wait_event! { |event| ... }` waits for one. Without a block both keep their native
  one-argument form.
* **`examples/rubyesque/`**, one small teaching script per slice of the layer (window, events,
  audio, input, system, deprecations) with an old-to-new cheat sheet in its README, plus the
  Rubyesque section of `README.md` and a per-module table in `ROADMAP.md`.
* **The graphics and audio class hierarchies now mirror SFML 3's own.** `SFML::Shape` is the base
  of `SFML::CircleShape`, `SFML::RectangleShape` and `SFML::ConvexShape`; `SFML::Sprite` and
  `SFML::Text` are `SFML::Transformable`; and `SFML::Sound`, `SFML::SoundStream` and `SFML::Music`
  derive from the new `SFML::SoundSource`, with `SFML::Music < SFML::SoundStream` and
  `SFML::SoundBufferRecorder < SFML::SoundRecorder`.

### Changed
* **The Rubyesque names are now primary**, with the pre-Rubyesque spellings kept as deprecated
  aliases that warn: `is_open?`, `focus?`, `request_focus`, `clear`, `display`,
  `play`/`pause`/`stop`, `Keyboard.pressed?`, `Joystick.has_axis?`, `Clipboard.string`/`string=`
  and `SFML.sleep`. Code written against the old names keeps working unchanged.
* **`SFML::Transformable` is now a module**, mixed into `Sprite`, `Text` and `Shape`, rather than a
  class. The position/rotation/scale/origin surface is implemented once and dispatched to each
  class's concrete CSFML entry point, replacing thirteen duplicated copies. `Transformable.new`
  still works and returns a `SFML::Transformable::Instance`, so standalone use and subclassing are
  preserved.
* **`SFML::Circle` is now `SFML::CircleShape`**, matching SFML's class name and its siblings. The
  old `SFML::Circle` name remains as a constant alias.

### Deprecated
* The pre-Rubyesque method names listed under **Changed**. They behave exactly as before and print a
  one-line warning pointing at the replacement; they will be removed in a future major release.

## [0.3.1] - 2026-09-19

### Added
* **`SFML::WindowBase`, `SFML::RenderWindow` and the `SFML::RenderTarget` module**, mirroring
  SFML 3's own hierarchy. `WindowBase` wraps `sfWindowBase` -- an OS window and event queue with
  no OpenGL context -- and is constructible, with `Window` and `RenderWindow` derived from it.
  `Window` keeps its existing `sfRenderWindow` behaviour; `RenderWindow` is the same renderable
  window under the name that also includes `RenderTarget`. `RenderTexture` includes
  `RenderTarget` too, so a drawable's `#draw` now accepts a `RenderWindow` or a `RenderTexture`
  directly (the legacy `Target` still works). The shared window surface is generated once, in
  `ext/window/window_base.inc`.
* **RBS signatures are now checked against the code.** `sig/**/*.rbs` declares `SFML::VERSION`, and
  a `Steepfile` plus the `steep` development gem add `rake steep` to type-check `lib/` against the
  signatures. `check.yaml` runs both `rake rbs` and `rake steep`, on `main` and on `release/**`
  PRs, which previously ran no workflow at all.

### Changed
* `Mouse.position`, `Mouse.set_position` and `Touch.position` dispatch to the
  `*RenderWindow` or `*WindowBase` CSFML entry point depending on whether the argument is a
  Window/RenderWindow or a WindowBase, instead of passing an `sfRenderWindow*` where an
  `sfWindowBase*` is expected.
* **RBS constructors use `def initialize`.** Every `def self.new` became
  `def initialize: (…) -> void`, the form RubyMine links to `Foo.new` and Steep checks, and which
  matches the C extension, where every constructor is an `initialize`.

## [0.3.0] - 2026-09-19

### Added
* **Four more experimental binary-gem targets**: `aarch64-linux-musl`, `arm-linux-gnu`
  (ARMv7 hard-float), `arm-linux-musl`, and `aarch64-mingw-ucrt` (64-bit Windows on ARM).
  All four ride the same rake-compiler-dock cross-compilation this project already uses for
  its other targets; none has been run on its target hardware yet, so all are
  `experimental` in `publish.yaml` like `aarch64-linux-gnu`, `x86_64-darwin` and
  `arm64-darwin` already were. `script/provision.sh` gained matching cases for the two new
  glibc/musl Linux targets.
* `rakelib/package.rake`'s `EXPECTED_ABIS` gained an entry for `aarch64-mingw-ucrt`: its
  rake-compiler-dock image carries no cross Ruby older than 3.4, so that gem has no floor
  below it — the same kind of upstream gap `x86-mingw32` already has at the top end.
* `aarch64-linux-gnu` is now additionally run — not just cross-compiled — by a new
  `test-arm64.yaml` workflow, on GitHub's hosted `ubuntu-24.04-arm` runner. It stays
  `experimental` in `publish.yaml` for now; this is the evidence that will eventually
  justify dropping that flag.
* ARMv6 (32-bit), RISC-V (rv64gc), PowerPC (ppc64le), and any BSD are deliberately not
  covered by a binary gem: rake-compiler-dock ships no cross-compilation image for any of
  them, and building custom cross-toolchain infrastructure for them is a much larger,
  separate undertaking. The source-gem fallback remains the only path there, unchanged.

### Fixed
* **A GC race in the audio bindings aborted the process on arm64.** `Sound`, `Music` and
  `SoundStream` released the GVL inside their `dfree`, but their data types were marked
  `RUBY_TYPED_FREE_IMMEDIATELY`, so the free could run *during* garbage collection: another
  thread (Ruby's `Timeout` thread, in the test suite) could then allocate while GC was mid-cycle
  and Ruby aborted with "object allocation during garbage collection phase". Dropping the flag
  defers the free to a safe point; releasing the GVL is still what keeps `sf*_destroy` from
  deadlocking against the audio thread.

### Documentation
* **Every public class, module, method and constant is documented**, with the
  result rendered on [rubydoc.info](https://rubydoc.info/gems/sfml3-rb). The
  comments live beside each binding in `ext/**/*.c`; `rake yard` builds them
  into `doc/`. Previously the comments carried `call-seq` and `@return` tags
  but little prose, so rubydoc.info showed a signature with a blank
  description; every such method now has a sentence of its own.
* RBS type signatures for the same surface ship in `sig/**/*.rbs`, for IDE
  completion (Solargraph, RubyMine) and static checking (Sorbet, Steep);
  `rake rbs` validates them.
* `.yardopts` ships in the gem, so rubydoc.info generates with the same title,
  README and extra files as `rake yard`.
* `rake doc:undoc` fails when a method is left without a description, so the
  coverage cannot regress silently (`yard stats` uses `blank?` and passes for
  tag-only comments, which is how the blanks went unnoticed).

## [0.2.1]

### Added
* **`SFML::Transform` is now a class**, wrapping `sfTransform` by value. It previously exposed
  only `combine` and `inverse` as module functions over plain 9-element Arrays, leaving 11 of
  CSFML's 13 transform functions unbound: you could read a matrix out of a `Sprite` but not build
  one, apply one to a point, or compose one. It now has `Transform.identity` / `IDENTITY`,
  `Transform.from_a`, `#to_a`/`#matrix`, `#gl_matrix`, `#==`, `#translate!`, `#rotate!`,
  `#scale!` (the last two taking an optional centre), non-mutating `#translate`/`#rotate`/
  `#scale`, `#transform_point`, `#transform_rect`, `#combine!`, `#*`, `#inverse` and `#copy`.
* **`map_pixel_to_coords` and `map_coords_to_pixel` on both render targets.** The screen-to-world
  conversion behind every click-to-select interaction; without it a zoomed, rotated or
  viewport-shifted `View` could not be hit-tested from Ruby at all.
* **`SFML::Window` gained the render-target methods only `RenderTexture` had**: `clear_stencil`,
  `clear_color_and_stencil`, `viewport`, `scissor` and `srgb?`. Both classes also gained
  `push_gl_states`, `pop_gl_states`, `reset_gl_states`, `draw_primitives` and
  `draw_vertex_buffer_range`. The shared surface is generated once from
  `ext/graphics/render_target.inc`, the same way `ext/audio/sound_source.inc` already serves the
  three sound sources.
* `SFML::View.from_rect`, `View#scissor` and `View#scissor=`.
* `SFML::Transformable#inverse_transform` and `#copy` — every subclass (`Sprite`, `Text`,
  `Circle`, `RectangleShape`, `ConvexShape`, `Shape`) already had both.
* `SFML::Texture.srgb`, `.srgb_from_stream`, `.srgb_from_image`, `Texture#resize_srgb` and
  `Texture#swap`.
* `SFML::Shader#set_vec4_array`, `#set_mat3_array` and `#set_mat4_array`, completing the array
  uniforms alongside the existing float/vec2/vec3 forms.
* `SFML::Circle#point_count=`, `SFML::VertexBuffer#swap`, `SFML::Window.from_handle` and
  `SFML::Window#create_vulkan_surface`.

### Fixed
* **Non-ASCII text and window titles were mangled.** `Text#string=`, `Window#title=`, window
  creation and `FtpDirectoryResponse#directory` passed UTF-8 bytes to CSFML's `const char*`
  entry points, which hand them to `sf::String`'s narrow-character constructor and decode them
  with the C locale: `"héllo"` came back as `U+0068 U+FFFFFFFF U+FFFFFFFF U+006C U+006C U+006F`.
  All four now go through the UTF-32 entry points and round-trip exactly, including astral-plane
  characters. The converters live in `ext/core/unicode.c`.
* **`Mouse.position`, `Mouse.position=` and `Touch.position` reinterpret-cast `sfRenderWindow*`
  to `sfWindowBase*`**, relying on the base being at offset 0 — true of CSFML 3.0.0's layout but
  not guaranteed by it. They now call `sfMouse_getPositionRenderWindow`,
  `sfMouse_setPositionRenderWindow` and `sfTouch_getPositionRenderWindow`, which CSFML ships for
  exactly this reason. Ruby-visible behaviour is unchanged.
* `Shader#set_float_array`, `#set_vec2_array` and `#set_vec3_array` leaked their conversion
  buffer if an element raised part-way through. All six array setters are now generated from one
  macro built on `ALLOCV_N`, whose buffer Ruby frees while unwinding.
* `VertexBuffer#update` had the same leak; it now shares `vertices_from_rb`, which validates
  every element before allocating anything.
* **Audio-thread callbacks re-entered the VM from a foreign native thread.** The
  `SoundStream#on_get_data` / `#on_seek` callbacks and the effect processor run on SFML's audio
  thread, which Ruby never created; calling back into Ruby from it (`rb_thread_call_with_gvl`) is
  a fatal VM error. Each now hands off to one shared Ruby-owned worker
  (`ext/core/foreign_thread.c`). `SoundSource#stop`, `Sound#buffer=` and the `Sound` / `Music` /
  `SoundStream` destructors release the GVL while they wait on the audio thread, and a processor
  that times out degrades to a full buffer of silence rather than stalling playback.

### Changed
* `Window` is created through `sfRenderWindow_createUnicode` rather than `sfRenderWindow_create`,
  so a title given at construction is subject to the same UTF-8 fix as `#title=`.
* `RenderTexture#viewport` and `#scissor` now take the view as an *optional* argument; omitting
  it means the target's current view, matching CSFML's `NULL`.

## [0.2.0]

### Added
* **Full Audio binding.** `SFML::Listener`, `SFML::SoundSourceCone`, `SFML::SoundBuffer`,
  `SFML::Sound`, `SFML::Music`, `SFML::SoundStream`, `SFML::SoundRecorder` and
  `SFML::SoundBufferRecorder`, plus the `SFML::SoundStatus` and `SFML::SoundChannel` enums. The
  shared sound-source surface (play/pause/stop/status, pitch, pan, volume, spatialization,
  position/direction/velocity, cone, doppler and attenuation factors, distance/gain bounds and
  playing offset) is generated once from `ext/audio/sound_source.inc` for all three source classes.
  `SoundStream` and `SoundRecorder` are subclassable through `#on_get_data`/`#on_seek` and
  `#on_process`/`#on_start`/`#on_stop`; their callbacks re-enter Ruby under the GVL and treat an
  exception as "stop", since they run on SFML's audio thread.
* **Full Network binding.** `SFML::IpAddress` (string/bytes/integer constructors, `NONE`, `ANY`,
  `LOCAL_HOST`, `BROADCAST`, local/public lookup), `SFML::Packet` (raw data plus every typed
  reader/writer), `SFML::SocketSelector`, `SFML::TcpSocket`, `SFML::TcpListener`,
  `SFML::UdpSocket`, `SFML::Http`/`HttpRequest`/`HttpResponse`, `SFML::Ftp` with its response,
  directory-response and listing-response classes, and the `SocketStatus`, `HttpMethod`,
  `HttpStatus`, `FtpStatus` and `FtpTransferMode` enums.
* `SFML::SoundSource#effect_processor=` accepts a Ruby proc. Because `sfEffectProcessor` carries
  no `userData` (so one C callback cannot tell which source invoked it), the binding dispatches
  through a bounded pool of C thunks, one per active source; exceeding the pool raises.

### Changed
* The vendored CSFML/SFML archives are now linked by **full path** instead of `-l`. mkmf puts the
  host's `-L/usr/lib64` ahead of the ports prefix, which made `-lfreetype` (and the new codec
  libraries) resolve to the host's shared objects and silently add runtime dependencies. The
  built extension again has no `libsfml`/`libcsfml`/`libfreetype`/`libvorbis`/`libFLAC`/`libogg`
  shared dependency.

### Build
* `ext/ports.rb` now builds **Ogg 1.3.5, Vorbis 1.3.7 and FLAC 1.4.3** as pinned,
  checksum-verified, static and position-independent ports, alongside FreeType. SFML 3's audio
  backend is miniaudio, so OpenAL is not needed.
* `SFML_BUILD_AUDIO`, `SFML_BUILD_NETWORK`, `CSFML_BUILD_AUDIO` and `CSFML_BUILD_NETWORK` are
  enabled; `ws2_32` was added to the Windows system libraries and the CoreAudio/CoreFoundation
  frameworks to the macOS ones. `CMakeLists.txt` finds and links the `Audio`/`Network` components.
* Archive extraction accepts `.tar.xz` as well as `.tar.gz`, since the FLAC release ships only as
  xz.

### Known issues
* Audio playback and capture need an audio device; the CI suite stays headless, so
  `SoundStream#on_get_data` and `SoundRecorder` are exercised by construction and mapping only,
  never by actually playing or capturing.
* `Http` and `Ftp` perform real network I/O and are not exercised in CI beyond construction and
  enum mapping.

## [0.1.1]

### Added
* **Full Graphics binding.** `RectangleShape`, `ConvexShape`, a callback-based custom `Shape`, `Sprite`,
  `Texture`, `Image`, `Font`, `Text`, `RenderTexture`, `Shader`, `Vertex`, `VertexArray`,
  `VertexBuffer`, `Glyph`, and the value types `BlendMode`, `StencilMode`. `Circle` gained the
  outline/texture/bounds surface it was missing.
* **Full Window/input binding.** `Keyboard` real-time queries (`pressed?`, `scancode_pressed?`,
  `localize`, `delocalize`, `description`), and the `Mouse`, `Joystick`, `Touch`, `Sensor`,
  `Clipboard`, `Cursor`, `Context`, `ContextSettings` and `Vulkan` modules/classes. `Event` now
  exposes every payload (text, mouse move/raw/button/wheel, joystick move/button/connect, touch,
  sensor).
* **System completeness.** `SFML::Time` (arithmetic, comparison, unit conversions), `SFML.sleep`,
  `SFML::Buffer`, and `SFML::InputStream` (wraps any `#read`-able object so `Font`/`Texture`/`Image`/
  `Shader` can load from streams).
* **Value classes.** `Vector2`, `Vector3`, `Color`, `Rect` are now real Ruby classes; every setter
  still accepts a plain array, and getters return the class (which behaves like an array via
  `to_ary`/`Enumerable`).
* `RenderTexture` is a draw target alongside `Window`: both route through the shared `Target`
  dispatcher, so every drawable renders to either.
* Predefined `Color` constants, `BlendMode`/`StencilMode` predefined values, `ContextSettings`,
  and `VideoMode.desktop_mode` / `.fullscreen_modes`.
* Initial project scaffolding: C extension wrapping CSFML (SFML 2 bindings).
* Windows, video modes, events, keyboard input, transforms, drawables (circle, rectangle, sprite, image), textures, views, clocks, and render targets.
* Unit tests (minitest) for Clock, Transformable, Circle, RenderState, VideoMode — headless, deterministic.
* RuboCop configuration and CI integration (test, check, publish workflows).
* Publish GitHub Action to build and upload the gem on release.
* `ext/ports.rb`: downloads, checksum-verifies, and builds SFML 3.0.2 and CSFML 3.0.0 from source into a static, position-independent prefix. Ships inside the gem, so `gem install sfml3-rb` builds its own dependencies with no CSFML pre-installed. Shared by `rake ports` (development) and `ext/extconf.rb` (install time).
* `ext/extconf.rb` three-way dependency resolution: `--enable-system-libraries` (or `SFML_USE_SYSTEM_LIBRARIES`) links a system CSFML 3; otherwise an existing `ports/<host>` prefix is reused, or built automatically.
* A compile-time `CSFML_VERSION_MAJOR < 3` guard (`ext/ext/sfml.h`) and an `extconf.rb` header check, so a CSFML 2.x install fails with one actionable message instead of a wall of compiler errors.
* `CMakeLists.txt` rewritten into a real, buildable configuration (CLion/IDE use only — `rake compile` remains the build of record): resolves the vendored `ports/<host>` prefix, links `find_package(SFML 3 ...)` and the CSFML static archives, and links through the C++ driver, since SFML is C++.

### Changed
* **Rebuilt the drawing path around `Target`.** `Target` now carries a `{type, handle}` union and
  dispatches each draw call to the window or render-texture variant; `Window#draw` and every
  drawable's `#draw` go through it. `RenderState` is initialized from `sfRenderStates_default`
  instead of field-by-field, fixing the `stencilMode`/`coordinateType` fields that were left
  uninitialized under CSFML 3, and it now exposes `blend_mode`, `stencil_mode`, `coordinate_type`,
  `texture` and `shader`.
* **Naming aligned with SFML 3, typos fixed.** `angle`/`angle=` → `rotation`/`rotation=`;
  `escalate` → `scale!` (the mutating operation; `scale` remains the getter); `matrix` → `transform`
  (`matrix` kept as an alias). `View#position`/`position=` were removed: the old setter wrote the
  viewport rather than the center — use `center`/`viewport`.
* `Clock#elapsed_time`/`#restart!` now return `SFML::Time` rather than bare `Float` seconds.
* `Window.new` accepts `(video_mode, title, style = :default, state = :windowed, settings = nil)`
  instead of hardcoding `sfDefaultStyle`/`sfWindowed`/no settings.
* **The C extension is now organised by SFML subsystem.** The 24 sources no longer sit flat in `ext/`
  with their headers in a doubled `ext/ext/` tree; each binding's `.c` and `.h` live together under
  `ext/core/`, `ext/system/`, `ext/window/` or `ext/graphics/`, matching how SFML and CSFML split
  themselves, so a binding's home follows from the CSFML header it wraps. Includes became
  subsystem-relative (`#include "graphics/circle.h"`) and header guards path-derived
  (`SFML_RB_GRAPHICS_CIRCLE_H`), retiring three `CANDY2D_EXT_*` guards left over from a previous
  project name. `ext/extconf.rb` sets `$srcs` from a recursive glob and extends `$VPATH`, because mkmf
  globs only the top level; objects stay flat, which is why every `.c` basename must remain unique.
  `Rakefile` gained `source_pattern = '**/*.{c,h}'` — recursive so the new tree is seen at all, and
  including headers because editing one previously triggered no rebuild. `CMakeLists.txt` globs
  recursively to match.
* `ext/ext/klass/transform.h` deleted: a stale duplicate of the live `module/transform.h` that nothing
  included, and which would have collided with it in the new tree. The five unbound placeholders
  (Image, Polygon, Rectangle, Sprite, Texture) were kept, so the layout still shows where they will
  land.
* `CrossBuild#build` now clears **every** staging directory under `pkg/`, not just the current
  platform's. `rake native:<platform> gem` also runs the shared source-gem task, so a stale
  `pkg/<name>-<version>/` fails the cross build with `... are not files` about files that plainly
  exist — which is exactly what the restructuring triggered.
* **The C extension moved from the untyped `Data_Wrap_Struct` family to `TypedData`.** All wrapped
  types now carry an `rb_data_type_t`. `Data_*` has been deprecated since Ruby 2.3 and warns on every
  use under Ruby 4.0, which was the extension's only compile-time portability problem — the build is
  now warning-clean under `-Wall -Wextra` from Ruby 3.1 to 4.0. `TypedData_Get_Struct` also
  type-checks, where `Data_Get_Struct` silently reinterpreted whatever pointer it was handed.
* `ext/extconf.rb` now always compiles with `-Wall -Wextra`, and with `-Werror=deprecated-declarations`
  when `SFML_STRICT=1`. CI sets it, so a regression to the deprecated API fails the build rather than
  adding noise. It is opt-in because an unfamiliar compiler at `gem install` time must never fail on a
  warning.
* **Minimum Ruby raised from 2.5.0 to 3.1**, matched by `TargetRubyVersion` in `.rubocop.yml` and by
  the binary gems, which is the point: there is now **one** floor rather than a source/binary split.
  `>= 2.5.0` had never been exercised — every workflow ran only Ruby 3.3. 3.1 is the oldest
  cross-ruby the `x64-mingw-ucrt` rake-compiler-dock image carries, so it is what lets a single
  `RUBY_CC_VERSION` serve every platform.
* `test.yaml` now runs the suite across 3.1, 3.2, 3.3, 3.4 and 4.0 — exactly the ABI set the binary
  gems ship — so every Ruby that can receive a precompiled gem is also proven to build from source,
  with `SFML_STRICT=1`.
* `publish.yaml` gained a `verify-source` gate that installs the freshly built source gem on the floor
  and requires `sfml`, so a release cannot ship a source gem that fails to build. Every supported
  Ruby now has a binary gem, but most *platforms* (macOS, ARM, the BSDs) still do not, and that path
  stays load-bearing.
* `Gemfile` gained a `:test` group pinning `minitest` to `~> 5.0`; minitest 6 requires Ruby >= 3.2,
  still above the floor. The cross-compile containers now install with `BUNDLE_WITHOUT="development
  test"`.
* `.rubocop.yml` excludes `pkg/`, `tmp/` and `ports/`. Those hold staged copies of `lib/` and `ext/`,
  so every offence in a real source file was being counted twice or more.
* **Packaging now produces precompiled binary gems** alongside the source gem. `rake gem:native`
  cross-compiles inside [rake-compiler-dock](https://github.com/rake-compiler/rake-compiler-dock);
  `rake platforms` lists the targets. On a covered platform `gem install sfml3-rb` no longer needs a
  toolchain, CMake, or a source build at all. The source gem remains the fallback everywhere else,
  unchanged in behaviour.
* **Binary gems now cover i686 as well as x86_64, on both glibc and musl Linux and on 32- and 64-bit
  Windows**: `x86-linux-gnu`, `x86-linux-musl` and `x86-mingw32` join `x86_64-linux-gnu`,
  `x86_64-linux-musl` and `x64-mingw-ucrt`, with `aarch64-linux-gnu`, `x86_64-darwin` and
  `arm64-darwin` still experimental. Two gaps are upstream, not ours: RubyInstaller publishes no
  32-bit Ruby 4.0, so `x86-mingw32` stops at 3.4; and 64-bit Windows before Ruby 3.1 is a different
  platform (`x64-mingw32`), which is not built.
* `rakelib/package.rake` asserts the ABIs of each gem it builds against `EXPECTED_ABIS`.
  rake-compiler only *warns* when an image lacks a cross-ruby it was asked for, so without this a gem
  could quietly ship fewer ABIs than intended.
* `ext/ports.rb` targets gained a `multiarch` key. i686 compiles with `i686-linux-gnu-gcc` but its
  libraries live in `/usr/lib/i386-linux-gnu`, and feeding the compiler triple to
  `CMAKE_LIBRARY_ARCHITECTURE` would have pointed CMake at a directory that does not exist.
* The extension build moved from a hand-rolled `rake compile` (`ruby extconf.rb && make` inside
  `ext/`, then a manual copy) to `rake-compiler`'s `Rake::ExtensionTask`. Builds are now out of tree
  in `tmp/`, so `ext/` no longer accumulates `.o` files, a `Makefile` and `mkmf.log` beside its
  sources.
* `lib/sfml.rb` prefers `sfml/<major.minor>/sfml_ext` — the per-ABI layout binary gems use — and falls
  back to `sfml/sfml_ext` for a source build.
* `ext/ports.rb` is now cross-aware: prefixes and build trees are keyed by target (`ports/<target>`)
  rather than by host, and a CMake toolchain file is generated per target. `SFML_TARGET` selects one;
  a native build is unaffected.
* Project renamed to sfml3.rb; **migrated from SFML 2 to SFML 3** (via CSFML 3) — no longer just
  planned. Distro CSFML packages are frequently still 2.x and are no longer supported.
* Native extension renamed from the bare `ext` to `sfml/sfml_ext`, matching the `sfml` gem namespace
  and what `rake-compiler` cross-compilation expects.
* `event_name.c` and `keyboard.c` name tables are now indexed by named enum constant (C99 designated
  initializers) instead of raw ordinal position, so a future upstream reorder is a compile error
  rather than a silently wrong event or key name.
* `Window#clear` now takes a single `[r, g, b, a]` color array, matching every other color-setting
  method in the API, instead of three positional numbers.
* Removed the dead `install-package` extconf hook and its accompanying `ext/linux.sh` (empty) and
  `ext/msys2.sh` stubs, and the unused `run_script`/`Arguments`/`library_nofound` helpers in
  `ext/auxlib.rb`.
* Removed the vendored `include/ruby/*.h` stubs and the empty `include/SFML/CSFML headers` placeholder;
  nothing referenced them once `CMakeLists.txt` was fixed to query the real Ruby headers.
* `sfml3-rb.gemspec` carries more information: a `documentation_uri` alongside the existing
  source/bug-tracker/changelog links, a summary that actually names SFML 3 and the precompiled
  binaries (it is the line `gem search` shows), and `extra_rdoc_files`/`rdoc_options`. It also ships
  `CHANGELOG.md` and `TODO.md` — the latter being the module-by-module record of what is bound, which
  is what tells someone whether the gem covers their case. Comments now record what a reader cannot
  infer: that rake-compiler rewrites `platform`, `required_ruby_version` and `extensions` for binary
  gems, and that `required_rubygems_version` is deliberately unset because pinning `>= 3.3.22` here
  would lock Ruby 3.1.0 (RubyGems 3.3.3) out of the source gem. Reformatted to 2-space indentation.
  No `homepage_uri`: it would duplicate `source_code_uri`, and RubyGems warns that only the first of a
  duplicated link is shown.
* `sfml.gemspec` → `sfml3-rb.gemspec`: description now describes the self-building install rather
  than a still-planned migration; license changed to `0BSD`, matching `LICENSE.md`.
* **Gem package renamed from `sfml` to `sfml3-rb`** (`gem install sfml3-rb`). `require 'sfml'` and the
  `SFML` Ruby module are unchanged — only the RubyGems package name moved.

### Fixed
* `Circle#scale` returned the position, not the scale.
* `RenderState` was missing a `dmark` slot; the wrapped struct now holds its texture/shader Ruby
  references so a GC cannot collect them while the states are in use.
* `View#position`/`position=` wrote the viewport under a misleading name (removed).
* `raise_method_no_implemented` crashed on `NULL` and never reached its message branch.
* `Window#joystick_threshold=` treated the threshold as a boolean.
* `Circle#draw` raised unconditionally: its second argument check tested `rb_target` against
  `RenderState` instead of `rb_state` (`ext/circle.c`), and a valid `Target` is never a `RenderState`,
  so no call could get past it.
* `Target#draw` unwrapped its `state` argument with no class check — the one remaining place a
  caller-supplied object reached a struct accessor unguarded. Combined with the untyped
  `Data_Get_Struct` it used to call, passing the wrong object there reinterpreted an arbitrary
  pointer; it now raises before that can happen.
* `ext/ext/module.h` and `ext/ext/klass/transform.h` each declared a `static VALUE` at file scope in a
  **header**, giving every one of the ~10 including translation units its own zero-initialized copy.
  `rb_mExt` moved into `ext.c`, its only user; `rb_cTransform` was dead and is gone.
* An unused `hash` local in `Event_key` (`ext/event.c`).
* Gem metadata: summary typo, empty description, incorrect homepage.
* Double-free crash (`free(): double free detected in tcache`) in `Circle`, `Clock`, `Transformable`,
  and `Window`: their `_free` functions called both CSFML's own `sfX_destroy` *and* `free()` on the
  same pointer, though `sfX_destroy` already releases it.
* `Event#size` read `event->size.height` for both components, so a resize event always reported a
  square. Also fixes the CSFML 3 struct layout (`sfSizeEvent.size` is now a vector, not flat
  `width`/`height`).
* `View#get_rotation` and `RenderTarget#draw` were missing `return` statements, so both handed an
  uninitialized `VALUE` back to Ruby — the same crash class as the double-free above, just not yet
  triggered by the test suite.
* `Transform.inverse` was bound to `Transform_combine` instead of `Transform_inverse` — a copy-paste
  bug that made `.inverse` silently wrong (and unusable at 1 argument).
* `Window#view=`, `#view`, and `#default_view` were fully implemented but never registered — calling
  them raised `NoMethodError`.
* CSFML 3 API port: video mode fields, `sfRenderWindow_create`'s new window-state parameter,
  `sfRenderWindow_waitEvent`'s new timeout parameter, and `sfFloatRect`'s `position`/`size` fields
  (was flat `left/top/width/height`).
* A `%ul` printf format-string bug in `ext/exceptions.c` produced garbled error messages (e.g.
  "given 3l"); corrected to `%lu`.
* `test/sfml_test.rb`: `require 'minitest/unit'` (removed from modern minitest) →
  `require 'minitest/autorun'`; classes referenced without their `SFML::` namespace;
  `assert_in_epsilon` used directly on arrays, which fails because it calls `.abs` on the expected
  value.

### Known issues
* Classes that wrap `T_DATA` never call `rb_define_alloc_func`, so Ruby prints `undefining the
  allocator of T_DATA class ...` once per class on first construction. Harmless and long-standing;
  fixing it means moving construction out of the singleton `new` into an alloc func plus
  `initialize`.
* Of the nine binary-gem targets, only `x86_64-linux-gnu` has been verified end to end (built,
  installed, full suite passes). `x64-mingw-ucrt` cross-builds and links cleanly with correct PE
  imports but has not been run on Windows. The rest have not been run on their target hardware;
  `aarch64-linux-gnu`, `x86_64-darwin` and `arm64-darwin` are marked `experimental` in
  `publish.yaml`, so a failure can't hold back a release.
* The Windows gem imports `libwinpthread-1.dll`, which RubyInstaller ships in its
  `ruby_builtin_dlls` directory. If that ever proves unreliable, add `-lwinpthread` to the
  `-Wl,-Bstatic` group in `Ports.cxx_runtime`.
* `bundle exec rubocop` does not pass on the repository as a whole (55 offenses across 14 files, 40
  autocorrectable, mostly `Style/FrozenStringLiteralComment`) — this predates the packaging work and
  `check.yaml` has been failing on it. The gemspec's own remaining offence is
  `Gemspec/RequireMFA`, left deliberately: setting `rubygems_mfa_required` requires MFA on every
  owner's RubyGems account, and `publish.yaml` pushes with an API token.
* [TODO.md](TODO.md) tracks SFML 3 → Ruby API porting coverage, module by module.