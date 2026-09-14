# Roadmap: SFML 3 API port to Ruby

Tracks porting coverage of the SFML 3 API into this gem, module by module and class by class.

This binding wraps **CSFML 3** (the C API), not SFML's C++ API directly, so the scope below is
CSFML 3.0.0's actual header set — not every C++-only construct in the
[reference docs](https://www.sfml-dev.org/documentation/3.1.0/annotated.html). Out of scope for that
reason: `String`/`Utf`/`Literals` (SFML C++ uses `std::string`; CSFML takes plain C strings),
`Exception` (CSFML reports errors via return codes, not C++ exceptions), and C++-utility-only types
(`SuspendAwareClock`, `TimeoutWithPredicate`, `U8StringCharTraits`). `Glsl` is handled as plain
`set_*_uniform` calls (its types are CSFML structs, not a templated class), and `InputStream`'s C
struct form *is* bound (`SFML::InputStream`). `Sftp` is also out of scope: it was added to SFML in
3.1, and the CSFML 3.0.0 this gem vendors predates it.

Status legend: `[x]` bound and tested · `[~]` compiled but not exposed to Ruby, or exposed as an
internal helper only · `[ ]` not started.

## System

Base module: time, vectors, clocks, streams.

- [x] **Clock** — `SFML::Clock` (`ext/system/clock.c`); `elapsed_time`, `restart!`, `reset!`,
      `start!`, `stop!`, `running?`, `copy`
- [x] **Vector2** — `SFML::Vector2` (`ext/system/vec2.c`); setters still accept `[x, y]`
- [x] **Vector3** — `SFML::Vector3` (`ext/system/vec3.c`)
- [x] **Time** — `SFML::Time` (`ext/system/time.c`); arithmetic, comparison, unit conversions
- [x] **Sleep** — `SFML.sleep` (`ext/system/sleep.c`)
- [x] **InputStream** — `SFML::InputStream` (`ext/system/input_stream.c`); wraps any `#read`-able
      object and is accepted by every `from_stream` constructor
- [x] **Buffer** — `SFML::Buffer` (`ext/system/buffer.c`); returned by `Image#save_to_memory`

## Window

OpenGL-based windows, events, input handling.

- [x] **Window** — `SFML::Window` (`ext/window/window.c`); wraps `sfRenderWindow`, so this single
      class covers what SFML splits into `WindowBase` + `Window` + `RenderWindow`. Accepts style,
      state and `ContextSettings`; exposes min/max size, icon, cursor, native handle, settings.
- [x] **VideoMode** — `SFML::VideoMode` (`ext/window/video_mode.c`); includes `desktop_mode` and
      `fullscreen_modes`
- [x] **Event** — `SFML::Event` (`ext/window/event.c`, `ext/window/event_name.c`); every payload is
      exposed — text, key, mouse move/raw/button/wheel, joystick move/button/connect, touch, sensor
- [x] **Keyboard** — `SFML::Keyboard` (`ext/window/keyboard.c`); real-time `pressed?`,
      `scancode_pressed?`, `localize`, `delocalize`, `description`, virtual-keyboard toggle
- [x] **Mouse** — `SFML::Mouse` (`ext/window/mouse.c`)
- [x] **Joystick** — `SFML::Joystick` (`ext/window/joystick.c`)
- [x] **Touch** — `SFML::Touch` (`ext/window/touch.c`)
- [x] **Sensor** — `SFML::Sensor` (`ext/window/sensor.c`)
- [x] **Clipboard** — `SFML::Clipboard` (`ext/window/clipboard.c`)
- [x] **Cursor** — `SFML::Cursor` (`ext/window/cursor.c`)
- [x] **Context** / **ContextSettings** — `SFML::Context`, `SFML::ContextSettings`
      (`ext/window/context.c`, `ext/window/context_settings.c`)
- [x] **Vulkan** — `SFML::Vulkan` (`ext/window/vulkan.c`)

## Graphics

2D rendering: shapes, sprites, text, render targets.

- [x] **Transform** — `SFML::Transform` (`ext/graphics/transform.c`)
- [x] **Transformable** — `SFML::Transformable` (`ext/graphics/transformable.c`)
- [x] **Drawable** — `SFML::Drawable` mixin (`ext/graphics/drawable.c`)
- [x] **RenderStates** — `SFML::RenderState` (`ext/graphics/render_state.c`); blend mode, stencil
      mode, coordinate type, texture, shader and transform are all settable
- [x] **RenderTarget** — `SFML::Target` (`ext/graphics/target.c`); dispatches to `sfRenderWindow_*`
      or `sfRenderTexture_*`
- [x] **RenderWindow** — folded into `SFML::Window` (see Window module above)
- [x] **RenderTexture** — `SFML::RenderTexture` (`ext/graphics/render_texture.c`)
- [x] **View** — `SFML::View` (`ext/graphics/view.c`)
- [x] **CircleShape** — `SFML::Circle` (`ext/graphics/circle.c`)
- [x] **RectangleShape** — `SFML::RectangleShape` (`ext/graphics/rectangle.c`)
- [x] **ConvexShape** — `SFML::ConvexShape` (`ext/graphics/polygon.c`)
- [x] **Shape** — `SFML::Shape` (`ext/graphics/shape.c`); subclass and define `point_count`/`point`
- [x] **Sprite** — `SFML::Sprite` (`ext/graphics/sprite.c`)
- [x] **Texture** — `SFML::Texture` (`ext/graphics/texture.c`)
- [x] **Image** — `SFML::Image` (`ext/graphics/image.c`)
- [x] **Font** — `SFML::Font` (`ext/graphics/font.c`)
- [x] **Text** — `SFML::Text` (`ext/graphics/text.c`)
- [x] **Glyph** — `SFML::Glyph` (`ext/graphics/glyph.c`)
- [x] **Shader** — `SFML::Shader` (`ext/graphics/shader.c`); scalar/vector/color/int/bool/matrix
      uniforms plus a generic `uniform=`
- [x] **Color** — `SFML::Color` (`ext/graphics/color.c`)
- [x] **Rect** — `SFML::Rect` (`ext/graphics/rect.c`); `sfFloatRect` and `sfIntRect`
- [x] **BlendMode** — `SFML::BlendMode` (`ext/graphics/blend_mode.c`)
- [x] **StencilMode** — `SFML::StencilMode` (`ext/graphics/stencil_mode.c`)
- [x] **Vertex** / **VertexArray** / **VertexBuffer** — `ext/graphics/vertex*.c`

## Audio

Sounds, streaming, recording, spatialization.

- [ ] **Not started — and not currently buildable.** `ext/ports.rb` builds the vendored SFML/CSFML
      with `SFML_BUILD_AUDIO=OFF` / `CSFML_BUILD_AUDIO=OFF`. Flipping those on (and pulling in the
      FLAC/Ogg/Vorbis dependency family they currently avoid) is a prerequisite for any class below.
- [ ] **Listener**
- [ ] **SoundBuffer**
- [ ] **Sound**
- [ ] **SoundBufferRecorder**
- [ ] **SoundRecorder**
- [ ] **SoundStream**
- [ ] **Music**
- [ ] **EffectProcessor**

## Network

Socket-based communication and higher-level protocols.

- [ ] **Not started — and not currently buildable.** `ext/ports.rb` builds the vendored SFML/CSFML
      with `SFML_BUILD_NETWORK=OFF` / `CSFML_BUILD_NETWORK=OFF`. Flipping those on is a prerequisite
      for any class below.
- [ ] **IpAddress**
- [ ] **Packet**
- [ ] **Socket** base / **SocketSelector**
- [ ] **TcpSocket** / **TcpListener**
- [ ] **UdpSocket**
- [ ] **Http**
- [ ] **Ftp**

## References

- [SFML 3.1.0 module topics](https://www.sfml-dev.org/documentation/3.1.0/topics.html)
- [SFML 3.1.0 class index](https://www.sfml-dev.org/documentation/3.1.0/annotated.html)
- [SFML 3.1.0 namespace index](https://www.sfml-dev.org/documentation/3.1.0/namespaces.html)
- [CSFML 3.0.0 headers](https://github.com/SFML/CSFML/tree/3.0.0/include/CSFML) — the actual C API
  surface this gem binds against, vendored locally under `ports/<target>/include/CSFML/`
  once `rake ports` has run
