# Roadmap: SFML 3 API port to Ruby

Tracks porting coverage of the SFML 3 API into this gem, module by module and class by class.

This binding wraps **CSFML 3** (the C API), not SFML's C++ API directly, so the scope below is
CSFML 3.0.0's actual header set — not every C++-only construct in the
[reference docs](https://www.sfml-dev.org/documentation/3.1.0/annotated.html). Out of scope for that
reason: `Glsl` (templated uniform helpers), `String`/`Utf`/`Literals` (SFML C++ uses `std::string`;
CSFML takes plain C strings), `InputStream` subclassing beyond its C struct form, `Exception`
(CSFML reports errors via return codes, not C++ exceptions), and C++-utility-only types
(`SuspendAwareClock`, `TimeoutWithPredicate`, `U8StringCharTraits`). `Sftp` is also out of scope: it
was added to SFML in 3.1, and the CSFML 3.0.0 this gem vendors predates it.

Status legend: `[x]` bound and tested · `[~]` compiled but not exposed to Ruby, or exposed as an
internal helper only · `[ ]` not started.

## System

Base module: time, vectors, clocks, streams.

- [x] **Clock** — `SFML::Clock` (`ext/clock.c`)
- [~] **Vector2** — internal conversion helper only (`ext/vec2.c`); points and sizes are passed as
      plain `[x, y]` Ruby arrays rather than a `Vector2` class
- [ ] **Vector3**
- [ ] **Time** — no dedicated class; `Clock#elapsed_time` returns a bare `Float` of seconds
- [ ] **InputStream** (loading from memory/custom sources rather than a file path)
- [ ] **Sleep**

## Window

OpenGL-based windows, events, input handling.

- [x] **Window** — `SFML::Window` (`ext/window.c`); wraps `sfRenderWindow`, so this single class
      covers what SFML splits into `WindowBase` + `Window` + `RenderWindow`
- [x] **VideoMode** — `SFML::VideoMode` (`ext/video_mode.c`)
- [x] **Event** — `SFML::Event` (`ext/event.c`, `ext/event_name.c`); closed, resized, key, and size
      events are implemented — text, mouse, joystick, touch, and sensor events are not (see below)
- [~] **Keyboard** — key-name lookup table only (`ext/keyboard.c`); no `SFML::Keyboard.pressed?`-style
      real-time query, and `KeyPressed`/`KeyReleased` events don't surface modifier state beyond what
      `Event#key` exposes
- [ ] **Mouse** — no real-time query and no `MouseMoved`/`MouseButtonPressed`/`MouseWheelScrolled`
      event support (`ext/event.c`'s mouse event methods are stubs returning `nil`)
- [ ] **Joystick** (real-time query and `JoystickMoved`/`JoystickButtonPressed`/`JoystickConnected`
      events)
- [ ] **Touch** (real-time query and `TouchBegan`/`TouchMoved`/`TouchEnded` events)
- [ ] **Sensor** (real-time query and `SensorChanged` events)
- [ ] **Clipboard**
- [ ] **Cursor** (custom cursor shapes)
- [ ] **Context** / **ContextSettings** (manual OpenGL context management)
- [ ] **Vulkan** (`sfVulkan_isAvailable`, extension query)

## Graphics

2D rendering: shapes, sprites, text, render targets.

- [x] **Transform** — `SFML::Transform` (`ext/transform.c`)
- [x] **Transformable** — `SFML::Transformable` (`ext/transformable.c`)
- [x] **Drawable** — `SFML::Drawable` mixin (`ext/drawable.c`)
- [x] **RenderStates** — `SFML::RenderState` (`ext/render_state.c`); blend mode is hardcoded to
      `sfBlendAlpha` rather than user-settable (see **BlendMode** below)
- [x] **RenderTarget** — `SFML::Target` (`ext/target.c`), the base drawing-surface abstraction
- [x] **RenderWindow** — folded into `SFML::Window` (see Window module above)
- [x] **View** — `SFML::View` (`ext/view.c`)
- [x] **CircleShape** — `SFML::Circle` (`ext/circle.c`) — note `#scale` currently returns position,
      not scale (tracked in [CHANGELOG.md](CHANGELOG.md))
- [~] **Color** — internal conversion helper only (`ext/color.c`); colors are passed as plain
      `[r, g, b, a]` Ruby arrays rather than an `SFML::Color` class
- [~] **Rect** (`sfFloatRect`/`sfIntRect`) — internal conversion helper only (`ext/rect.c`); rects are
      passed as plain 4-element arrays
- [ ] **RectangleShape** — file exists but is an empty stub (`ext/rectangle.c`)
- [ ] **ConvexShape** — file exists but is an empty stub (`ext/polygon.c`)
- [ ] **Sprite** — file exists but is an empty stub (`ext/sprite.c`)
- [ ] **Texture** — file exists but is an empty stub (`ext/texture.c`)
- [ ] **Image** — file exists but is an empty stub (`ext/image.c`)
- [ ] **Shape** (the shared base that `RectangleShape`/`ConvexShape`/`CircleShape` extend — worth
      revisiting once the stub shapes above are implemented, so outline/texture logic isn't repeated
      per shape)
- [ ] **RenderTexture** (off-screen rendering)
- [ ] **Font**
- [ ] **Text**
- [ ] **Glyph**
- [ ] **Shader** (vertex/geometry/fragment)
- [ ] **BlendMode** (as a settable type — see RenderStates above)
- [ ] **StencilMode**
- [ ] **Vertex** / **VertexArray** / **VertexBuffer**

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
  surface this gem binds against, vendored locally under `ports/build/CSFML-3.0.0/include/CSFML/`
  once `rake ports` has run
