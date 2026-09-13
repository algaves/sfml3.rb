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
* `ext/ports.rb`: downloads, checksum-verifies, and builds SFML 3.0.2 and CSFML 3.0.0 from source into a static, position-independent prefix. Ships inside the gem, so `gem install sfml` builds its own dependencies with no CSFML pre-installed. Shared by `rake ports` (development) and `ext/extconf.rb` (install time).
* `ext/extconf.rb` three-way dependency resolution: `--enable-system-libraries` (or `SFML_USE_SYSTEM_LIBRARIES`) links a system CSFML 3; otherwise an existing `ports/<host>` prefix is reused, or built automatically.
* A compile-time `CSFML_VERSION_MAJOR < 3` guard (`ext/ext/sfml.h`) and an `extconf.rb` header check, so a CSFML 2.x install fails with one actionable message instead of a wall of compiler errors.
* `CMakeLists.txt` rewritten into a real, buildable configuration (CLion/IDE use only — `rake compile` remains the build of record): resolves the vendored `ports/<host>` prefix, links `find_package(SFML 3 ...)` and the CSFML static archives, and links through the C++ driver, since SFML is C++.

### Changed
* Project renamed to sfml3.rb; **migrated from SFML 2 to SFML 3** (via CSFML 3) — no longer just planned. Distro CSFML packages are frequently still 2.x and are no longer supported.
* Native extension renamed from the bare `ext` to `sfml/sfml_ext`, matching the `sfml` gem namespace and what `rake-compiler` cross-compilation expects.
* `event_name.c` and `keyboard.c` name tables are now indexed by named enum constant (C99 designated initializers) instead of raw ordinal position, so a future upstream reorder is a compile error rather than a silently wrong event or key name.
* `Window#clear` now takes a single `[r, g, b, a]` color array, matching every other color-setting method in the API, instead of three positional numbers.
* Removed the dead `install-package` extconf hook and its accompanying `ext/linux.sh` (empty) and `ext/msys2.sh` stubs, and the unused `run_script`/`Arguments`/`library_nofound` helpers in `ext/auxlib.rb`.
* Removed the vendored `include/ruby/*.h` stubs and the empty `include/SFML/CSFML headers` placeholder; nothing referenced them once `CMakeLists.txt` was fixed to query the real Ruby headers.
* `sfml.gemspec`: description now describes the self-building install rather than a still-planned migration; license changed to `0BSD`, matching `LICENSE.md`.

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
* Precompiled native gems are not yet in place, and `.github/workflows/publish.yaml` still builds a source-only gem with a broken `gem push` step (see `test.yaml`/`publish.yaml`).
* [TODO.md](TODO.md) tracks SFML 3 → Ruby API porting coverage, module by module.