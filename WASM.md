# WebAssembly (Emscripten) support

Working notes for the `development/port/wasm` branch. This is the spike record
for Milestone 0 of the wasm effort; it will grow into the user-facing guide.

## Goal

Run `sfml3-rb` in a browser, with the game loop driven from Ruby:

```ruby
SFML.emscripten_loop do
  # game loop in SFML
end if SFML.platform == :wasm
```

## Blocker to be solved first

**Upstream SFML 3.0.2 and CSFML 3.0.0 have no Emscripten backend.** The pinned
sources under `ports/build/<target>/SFML-3.0.2/` have no
`src/SFML/Window/Emscripten/` directory and `cmake/Config.cmake` `FATAL_ERROR`s
on any OS other than Windows/Linux/BSD/macOS/iOS/Android. This must be added
(a vendored patch under `ports/patches/`) before anything can link. See the plan
in the branch history for the full phased port.

## Pinned toolchain (validated)

| Component | Version | Notes |
| --- | --- | --- |
| Emscripten SDK | `3.1.54` | matches ruby.wasm's emscripten builder image (`emscripten/emsdk:3.1.54`) |
| Ruby (ruby.wasm cross build) | `3.4.1` | `rbwasm build --ruby-version 3.4` |
| `ruby_wasm` gem / `rbwasm` | `2.10.1` | |
| Ruby target triple | `wasm32-unknown-emscripten` | required for `emscripten_set_main_loop_arg` |
| Node (loader) | `>= 20` | ruby.wasm's loader uses ESM/`createRubyModule` |

Container used for the spike (derived from ruby.wasm's own builder):

```
FROM ghcr.io/ruby/ruby.wasm/builder/wasm32-unknown-emscripten:main
ENV PATH=/opt/ruby/bin:/usr/local/cargo/bin:$PATH
RUN gem install ruby_wasm --no-document
ENTRYPOINT ["/bin/bash"]
```

## Build command (validated)

Native extensions are only linked when `rbwasm` runs under Bundler **with the
extension gem in the `Gemfile`**. Run standalone, `defined?(Bundler)` is nil and
`rbwasm` silently skips `spec.extensions` — no error, just no extension in the
result. So the wasm build must be:

```
bundle install
bundle exec rbwasm build \
  --target wasm32-unknown-emscripten \
  --ruby-version 3.4 \
  --build-profile minimal \
  --remake \
  -o dist/ruby.wasm
```

Extra Emscripten link flags go through `RUBY_WASM_EMCC_LDFLAGS`
(`RubyWasm::CrossRubyProduct#configure_args` appends it to the emscripten
`ldflags`, which already contain `-s MODULARIZE=1`). The spike used:

```
RUBY_WASM_EMCC_LDFLAGS="-sASYNCIFY -sALLOW_MEMORY_GROWTH=1"
```

## Artifact layout (validated)

`rbwasm build -o <name>` for the emscripten target writes the Emscripten **JS
glue** into `<name>` — it reads `usr/local/bin/ruby` (the glue), not the actual
wasm. The real artifacts live in the rubies tree:

```
rubies/ruby-3.4-wasm32-unknown-emscripten-minimal[-<hash>]/
  usr/local/bin/ruby        # Emscripten MODULARIZE JS glue (~116 KB)
  usr/local/bin/ruby.wasm   # the wasm binary (~9.3 MB)
  usr/local/lib/...         # stdlib + installed gem Ruby files
```

The `-<hash>` suffix appears when the Gemfile has a native extension
(`StaticLinking#name` hashes `spec.full_name`). The wasm contains the extension:
`strings ruby.wasm | grep SpikeEmLoop` finds it.

To run in a browser, follow ruby.wasm's
`packages/npm-packages/ruby-wasm-emscripten/build-package.sh`: copy the glue to
`ruby.js`, the binary to `ruby.wasm`, generate `ruby_stdlib.data`/`.js` with
`file_packager` preloading `usr/local/lib@/usr/local/lib`, and load via the
`@ruby/wasm-emscripten` pattern (`globalThis.__ruby_module = Module;
createRubyModule(Module)`).

## `emscripten_set_main_loop_arg` (validated)

A minimal native extension that calls
`emscripten_set_main_loop_arg(cb, NULL, 0, 1)` from a Ruby module function,
retains the block via `rb_gc_register_address`, and runs it in the callback
works end-to-end in Node:

```
[err] spike: before loop
spike: module ready
[err] spike: frame      # x3 frames
NODE EXIT=0
```

Findings:

- The Ruby block runs once per frame; `emscripten_cancel_main_loop()` stops the
  loop and the Node process exits cleanly.
- Code after `emscripten_set_main_loop_arg` does **not** resume, even with
  `-sASYNCIFY`. Treat the loop as terminal: the whole game lives inside the
  block, matching SFML's own C++ examples.
- `emscripten_set_main_loop_arg` is available to the extension because the
  Emscripten runtime provides it; no extra library to link.

## Open items carried into the implementation milestones

- **SFML/CSFML Emscripten backend** — the critical path (see the plan).
- **Gem Ruby-side packaging**: `make install-rb` did not place the spike gem's
  `lib/*.rb` into the wasm filesystem, so `require 'sfml'` will need the gem's
  Ruby files preloaded explicitly (file_packager / bundler `setup.rb`), separate
  from the statically linked C extension. The C extension itself is reachable
  via its feature name (`require 'sfml/sfml_ext'`).
- **`-sASYNCIFY`** appears necessary in practice for the main loop; confirm the
  exact flag set against the real SFML build.
- **`wasm32-emscripten` vs `wasm32-wasi` paths**: the emscripten build installs
  under `usr/local/lib/ruby/3.4.0/wasm32-emscripten`, while some ruby.wasm
  internals glob `wasm32-wasi` (notably `CrossRubyProduct#rbconfig_rb`, only used
  on the pic/dynamic path). The non-pic emscripten build used here is unaffected,
  but watch this if dynamic linking is ever enabled.
- **Network** under Emscripten is restricted to WebSocket/XHR; raw TCP listeners
  are not available without a proxy.
