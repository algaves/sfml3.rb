# sfml3-rb in the browser (WebAssembly)

A headless demo of `sfml3-rb` compiled to `wasm32-unknown-emscripten` and
statically linked into `ruby.wasm`, served over plain HTTP.

This build covers **System + Audio only**. Upstream SFML 3.0.2 has no Emscripten
window/graphics backend yet, so there is no window or canvas to render; the page
runs the assertions in `tests.rb` and prints them, ending with
`ALL SFML WASM TESTS PASS`. See `WASM.md` at the repository root for the
toolchain pins, the porting plan, and the still-open Window/Graphics work.

## Run

The generated wasm artifacts are **not** committed (they are ~30 MB together).
Once they have been built into this directory —

```
examples/web/ruby.js
examples/web/ruby.wasm
examples/web/ruby_stdlib.js
examples/web/ruby_stdlib.data
```

— serve the directory over HTTP. A `file://` page will not work: the loader
fetches the `.data` file with XHR, which browsers block on `file://`.

```
python3 -m http.server 8000 --bind 127.0.0.1
# then open http://127.0.0.1:8000/
```

`index.html` sets up the Emscripten `Module` instance, loads the file-packaged
stdlib, runs the glue inside a function scope (so its top-level `var Module`
cannot clobber the instance), and calls the exported module factory.

## How the artifacts are built

The pinned toolchain is `emcc` 3.1.54 with the ruby.wasm Emscripten builder
(`ruby_wasm` 2.10.1, Ruby 3.4.1), building the extension with
`bundle exec rbwasm build --target wasm32-unknown-emscripten`. The SFML/CSFML
wasm prefix is built from the patches in `script/wasm/patches/`. See `WASM.md`
for the full, validated recipe and the traps (`rbwasm` must run under Bundler;
`-o` writes the JS glue, not the wasm; the loop is terminal).
