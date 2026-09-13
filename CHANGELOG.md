# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
* Initial project scaffolding: C extension wrapping CSFML (SFML 2 bindings).
* Windows, video modes, events, keyboard input, transforms, drawables (circle, rectangle, sprite, image), textures, views, clocks, and render targets.
* Unit tests (minitest) for Clock, Transformable, Circle, RenderState, VideoMode — headless, deterministic; avoid Circle#scale due to a known bug.
* RuboCop configuration and CI integration (test, check, publish workflows).
* Publish GitHub Action to build and upload the gem on release.
* `ext/ports.rb`: downloads, checksum-verifies, and builds SFML 3.0.2 and CSFML 3.0.0 from source into a static, position-independent prefix. Ships inside the gem, so `gem install sfml3-rb` builds its own dependencies with no CSFML pre-installed. Shared by `rake ports` (development) and `ext/extconf.rb` (install time).
* `ext/extconf.rb` three-way dependency resolution: `--enable-system-libraries` (or `SFML_USE_SYSTEM_LIBRARIES`) links a system CSFML 3; otherwise an existing `ports/<host>` prefix is reused, or built automatically.
* A compile-time `CSFML_VERSION_MAJOR < 3` guard (`ext/ext/sfml.h`) and an `extconf.rb` header check, so a CSFML 2.x install fails with one actionable message instead of a wall of compiler errors.
* `CMakeLists.txt` rewritten into a real, buildable configuration (CLion/IDE use only — `rake compile` remains the build of record): resolves the vendored `ports/<host>` prefix, links `find_package(SFML 3 ...)` and the CSFML static archives, and links through the C++ driver, since SFML is C++.

### Changed
* **Packaging now produces precompiled binary gems** alongside the source gem. `rake gem:native` cross-compiles for `x86_64-linux-gnu`, `x64-mingw-ucrt`, `aarch64-linux-gnu`, `x86_64-linux-musl`, `x86_64-darwin` and `arm64-darwin` inside [rake-compiler-dock](https://github.com/rake-compiler/rake-compiler-dock); `rake platforms` lists them. On a covered platform `gem install sfml3-rb` no longer needs a toolchain, CMake, or a source build at all. The source gem remains the fallback everywhere else, unchanged in behaviour.
* The extension build moved from a hand-rolled `rake compile` (`ruby extconf.rb && make` inside `ext/`, then a manual copy) to `rake-compiler`'s `Rake::ExtensionTask`. Builds are now out of tree in `tmp/`, so `ext/` no longer accumulates 23 `.o` files, a `Makefile` and `mkmf.log` beside its sources.
* `lib/sfml.rb` prefers `sfml/<major.minor>/sfml_ext` — the per-ABI layout binary gems use — and falls back to `sfml/sfml_ext` for a source build.
* `ext/ports.rb` is now cross-aware: prefixes and build trees are keyed by target (`ports/<target>`) rather than by host, and a CMake toolchain file is generated per target. `SFML_TARGET` selects one; a native build is unaffected.
* Project renamed to sfml3.rb; **migrated from SFML 2 to SFML 3** (via CSFML 3) — no longer just planned. Distro CSFML packages are frequently still 2.x and are no longer supported.
* Native extension renamed from the bare `ext` to `sfml/sfml_ext`, matching the `sfml` gem namespace and what `rake-compiler` cross-compilation expects.
* `event_name.c` and `keyboard.c` name tables are now indexed by named enum constant (C99 designated initializers) instead of raw ordinal position, so a future upstream reorder is a compile error rather than a silently wrong event or key name.
* `Window#clear` now takes a single `[r, g, b, a]` color array, matching every other color-setting method in the API, instead of three positional numbers.
* Removed the dead `install-package` extconf hook and its accompanying `ext/linux.sh` (empty) and `ext/msys2.sh` stubs, and the unused `run_script`/`Arguments`/`library_nofound` helpers in `ext/auxlib.rb`.
* Removed the vendored `include/ruby/*.h` stubs and the empty `include/SFML/CSFML headers` placeholder; nothing referenced them once `CMakeLists.txt` was fixed to query the real Ruby headers.
* `sfml.gemspec` → `sfml3-rb.gemspec`: description now describes the self-building install rather than a still-planned migration; license changed to `0BSD`, matching `LICENSE.md`.
* **Gem package renamed from `sfml` to `sfml3-rb`** (`gem install sfml3-rb`). `require 'sfml'` and the `SFML` Ruby module are unchanged — only the RubyGems package name moved.

### Fixed
* Gem metadata: summary typo, empty description, incorrect homepage.
* Double-free crash (`free(): double free detected in tcache`) in `Circle`, `Clock`, `Transformable`, and `Window`: their `_free` functions called both CSFML's own `sfX_destroy` *and* `free()` on the same pointer, though `sfX_destroy` already releases it.
* `Event#size` read `event->size.height` for both components, so a resize event always reported a square. Also fixes the CSFML 3 struct layout (`sfSizeEvent.size` is now a vector, not flat `width`/`height`).
* `View#get_rotation` and `RenderTarget#draw` were missing `return` statements, so both handed an uninitialized `VALUE` back to Ruby — the same crash class as the double-free above, just not yet triggered by the test suite.
* `Transform.inverse` was bound to `Transform_combine` instead of `Transform_inverse` — a copy-paste bug that made `.inverse` silently wrong (and unusable at 1 argument).
* `Window#view=`, `#view`, and `#default_view` were fully implemented but never registered — calling them raised `NoMethodError`.
* CSFML 3 API port: video mode fields, `sfRenderWindow_create`'s new window-state parameter, `sfRenderWindow_waitEvent`'s new timeout parameter, and `sfFloatRect`'s `position`/`size` fields (was flat `left/top/width/height`).
* A `%ul` printf format-string bug in `ext/exceptions.c` produced garbled error messages (e.g. "given 3l"); corrected to `%lu`.
* `test/sfml_test.rb`: `require 'minitest/unit'` (removed from modern minitest) → `require 'minitest/autorun'`; classes referenced without their `SFML::` namespace; `assert_in_epsilon` used directly on arrays, which fails because it calls `.abs` on the expected value.

### Known issues
* `Circle#scale` returns the shape's position, not its scale (`ext/circle.c`).
* Of the six binary-gem targets, only `x86_64-linux-gnu` has been verified end to end (built, installed, full suite passes). `x64-mingw-ucrt` cross-builds and links cleanly with correct PE imports but has not been run on Windows. `aarch64-linux-gnu`, `x86_64-linux-musl`, `x86_64-darwin` and `arm64-darwin` have not been built yet and are marked `experimental` in `publish.yaml`, so a failure can't hold back a release.
* The Windows gem imports `libwinpthread-1.dll`, which RubyInstaller ships in its `ruby_builtin_dlls` directory. If that ever proves unreliable, add `-lwinpthread` to the `-Wl,-Bstatic` group in `Ports.cxx_runtime`.
* `bundle exec rubocop` does not pass on the repository as a whole (88 offenses, mostly `Style/FrozenStringLiteralComment` and the gemspec's 4-space indentation) — this predates the packaging work and `check.yaml` has been failing on it.
* [TODO.md](TODO.md) tracks SFML 3 → Ruby API porting coverage, module by module.