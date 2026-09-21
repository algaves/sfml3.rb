# Roadmap: SFML 3 API port to Ruby

Tracks porting coverage of the SFML 3 API into this gem, module by module and class by class, plus
what is still ahead of it. Coverage is measured against **CSFML 3.0.0** — the C API this gem
actually links — while the missing-elements list at the bottom is read against **SFML 3.1.0**, the
latest upstream release.

This binding wraps **CSFML 3** (the C API), not SFML's C++ API directly, so the scope below is
CSFML 3.0.0's actual header set — not every C++-only construct in the
[reference docs](https://www.sfml-dev.org/documentation/3.1.0/annotated.html). Out of scope for that
reason: `String`/`Utf`/`Literals` (SFML C++ uses `std::string`; CSFML takes plain C strings),
`Exception` (CSFML reports errors via return codes, not C++ exceptions), and C++-utility-only types
(`SuspendAwareClock`, `TimeoutWithPredicate`, `U8StringCharTraits`). `Glsl` is handled as plain
`set_*_uniform` calls (its types are CSFML structs, not a templated class), and `InputStream`'s C
struct form *is* bound (`SFML::InputStream`). Types that SFML 3 models as dedicated C++ classes but
CSFML models as plain C scalars need no class of their own here: `Angle` is a `Float` (degrees),
`StencilValue` is an `Integer`, `RenderStates` is `SFML::RenderState`, and the vector types are
plain `[x, y]` / `[x, y, z]` arrays.

Status legend: `[x]` bound and tested · `[~]` compiled but not exposed to Ruby, or exposed as an
internal helper only · `[ ]` not started · `[!]` exists in SFML upstream but has no CSFML entry
point yet (blocked on a CSFML release).

**Coverage is tracked per function, not per class.** Every `sf*` entry point in
`ports/<target>/include/CSFML` is accounted for: bound, or listed in "Deliberately unbound" at the
bottom. Re-derive the list with:

```sh
grep -rhoE '\bsf[A-Za-z0-9_]+\s*\(' ports/<target>/include/CSFML --include='*.h' \
  | tr -d ' (' | sort -u > /tmp/api.txt
grep -rhoE '\bsf[A-Za-z0-9_]+' ext --include='*.c' --include='*.h' --include='*.inc' \
  | sort -u > /tmp/used.txt
comm -23 /tmp/api.txt /tmp/used.txt
```

Mind the false positives: functions reached through token paste (`sfSound_*`, `sfMusic_*`,
`sfSoundStream_*` from `ext/audio/sound_source.inc`; `sfPacket_read*`/`write*` from the macro in
`ext/network/packet.c`; the shared `sfRenderWindow_*`/`sfRenderTexture_*` render-target methods
from `ext/graphics/render_target.inc`) never appear literally in the sources and so show up as
"missing" every time.

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

- [x] **WindowBase** — `SFML::WindowBase` (`ext/window/window_base.c`); wraps `sfWindowBase`, an OS
      window and event queue with no OpenGL context. The methods shared with `Window` are generated
      from `ext/window/window_base.inc`.
- [x] **Window** — `SFML::Window` (`ext/window/window.c`); wraps `sfRenderWindow` and derives from
      `WindowBase`, overriding every base method with the matching `sfRenderWindow_*` entry point.
      Accepts style, state and `ContextSettings`; exposes min/max size, icon, cursor, native handle,
      settings, display, and the render-target surface. Deliberately still renderable, so existing
      `Window.new(...).clear` code keeps working.
- [x] **VideoMode** — `SFML::VideoMode` (`ext/window/video_mode.c`); includes `desktop_mode` and
      `fullscreen_modes`
- [x] **Event** — `SFML::Event` (`ext/window/event.c`, `ext/window/event_name.c`); every payload is
      exposed — text, key, mouse move/raw/button/wheel, joystick move/button/connect, touch, sensor
- [x] **Keyboard** — `SFML::Keyboard` (`ext/window/keyboard.c`); real-time `pressed?`,
      `scancode_pressed?`, `localize`, `delocalize`, `description`, virtual-keyboard toggle
- [x] **Mouse** — `SFML::Mouse` (`ext/window/mouse.c`)
- [x] **Joystick** — `SFML::Joystick` (`ext/window/joystick.c`); includes `identification`
- [x] **Touch** — `SFML::Touch` (`ext/window/touch.c`)
- [x] **Sensor** — `SFML::Sensor` (`ext/window/sensor.c`)
- [x] **Clipboard** — `SFML::Clipboard` (`ext/window/clipboard.c`)
- [x] **Cursor** — `SFML::Cursor` (`ext/window/cursor.c`)
- [x] **Context** / **ContextSettings** — `SFML::Context`, `SFML::ContextSettings`
      (`ext/window/context.c`, `ext/window/context_settings.c`)
- [x] **Vulkan** — `SFML::Vulkan` (`ext/window/vulkan.c`)

## Graphics

2D rendering: shapes, sprites, text, render targets.

- [x] **Transform** — `SFML::Transform` (`ext/graphics/transform.c`); a class wrapping
      `sfTransform` by value. `identity`/`IDENTITY` (frozen), `from_a`, `to_a`/`matrix` (3x3),
      `gl_matrix` (the 16-float 4x4 for `glLoadMatrixf`), `==`, `translate!`, `rotate!`, `scale!`
      (both taking an optional centre), non-mutating `translate`/`rotate`/`scale`,
      `transform_point`, `transform_rect`, `combine!`, `*`, `inverse`, `copy`. The old module
      functions `Transform.combine`/`Transform.inverse` remain, Array-in/Array-out. Every class
      that exposes `#transform` still returns a plain 9-element Array, and anything that *takes*
      a transform accepts either form
- [x] **Transformable** — `SFML::Transformable` (`ext/graphics/transformable.c`); a **module**
      (Ruby has no multiple inheritance, so every drawable mixes it in) providing position,
      rotation, scale, origin, `move`/`rotate`/`scale!`, `transform`/`matrix`/`inverse_transform`.
      The methods are implemented once and dispatched to the receiver's concrete `sf*` entry point;
      the standalone object is `SFML::Transformable::Instance` (returned by `Transformable.new`;
      internal, not part of the public surface)
- [x] **Drawable** — `SFML::Drawable` mixin (`ext/graphics/drawable.c`)
- [x] **RenderStates** — `SFML::RenderState` (`ext/graphics/render_state.c`); blend mode, stencil
      mode, coordinate type, texture, shader and transform are all settable
- [x] **RenderTarget** — `SFML::RenderTarget` module (`ext/graphics/target.c`), included by
      `RenderWindow` and `RenderTexture`, so a drawable's `#draw` accepts either directly. The
      methods — `map_pixel_to_coords`, `map_coords_to_pixel`, `push_gl_states`, `pop_gl_states`,
      `reset_gl_states`, `draw_primitives`, `draw_vertex_buffer_range`, `clear_stencil`,
      `clear_color_and_stencil`, `viewport`, `scissor`, `srgb?` — are generated once for both
      `Window` and `RenderTexture` from `ext/graphics/render_target.inc`. `SFML::Target` remains
      as the legacy runtime-dispatch wrapper for `Drawable#draw`.
- [x] **RenderWindow** — `SFML::RenderWindow` (`ext/graphics/render_window.c`); derives from
      `Window` and includes `RenderTarget`. Creation, events and the window surface come from
      `Window` / `WindowBase`.
- [x] **RenderTexture** — `SFML::RenderTexture` (`ext/graphics/render_texture.c`)
- [x] **View** — `SFML::View` (`ext/graphics/view.c`); including `View.from_rect` and
      `scissor`/`scissor=`
- [x] **CircleShape** — `SFML::CircleShape` (`ext/graphics/circle.c`), also available as the
      `SFML::Circle` alias; derives from `Shape`
- [x] **RectangleShape** — `SFML::RectangleShape` (`ext/graphics/rectangle.c`); derives from `Shape`
- [x] **ConvexShape** — `SFML::ConvexShape` (`ext/graphics/polygon.c`); derives from `Shape`
- [x] **Shape** — `SFML::Shape` (`ext/graphics/shape.c`); the base of the built-in shapes, mixing in
      `Transformable` and `Drawable`. Subclass it and define `point_count`/`point` for a custom shape
- [x] **Sprite** — `SFML::Sprite` (`ext/graphics/sprite.c`); mixes in `Transformable` and `Drawable`
- [x] **Texture** — `SFML::Texture` (`ext/graphics/texture.c`); every constructor in both linear
      and sRGB form, plus `resize`/`resize_srgb` and `swap`
- [x] **Image** — `SFML::Image` (`ext/graphics/image.c`)
- [x] **Font** — `SFML::Font` (`ext/graphics/font.c`); `info` returns the family name
- [x] **Text** — `SFML::Text` (`ext/graphics/text.c`); mixes in `Transformable` and `Drawable`;
      `#string` goes through the UTF-32 entry
      points, so non-ASCII round-trips exactly
- [x] **Glyph** — `SFML::Glyph` (`ext/graphics/glyph.c`)
- [x] **Shader** — `SFML::Shader` (`ext/graphics/shader.c`); scalar/vector/color/int/bool/matrix
      uniforms, all six array uniforms (float, vec2-4, mat3, mat4), `set_current_texture`, plus a
      generic `uniform=`
- [x] **Color** — `SFML::Color` (`ext/graphics/color.c`)
- [x] **Rect** — `SFML::Rect` (`ext/graphics/rect.c`); `sfFloatRect` and `sfIntRect`
- [x] **BlendMode** — `SFML::BlendMode` (`ext/graphics/blend_mode.c`)
- [x] **StencilMode** — `SFML::StencilMode` (`ext/graphics/stencil_mode.c`)
- [x] **Vertex** / **VertexArray** / **VertexBuffer** — `ext/graphics/vertex*.c`

## Audio

Sounds, streaming, recording, spatialization.

- [x] **Listener** — `SFML::Listener` (`ext/audio/listener.c`); global volume plus
      position/direction/velocity/up-vector/cone
- [x] **ListenerCone** / **SoundSourceCone** — `SFML::SoundSourceCone` (`ext/audio/sound_source_cone.c`)
- [x] **SoundSource** — `SFML::SoundSource` (`ext/audio/sound_source.c`), the base class of
      `Sound`, `SoundStream` and `Music`; the shared play/pause/stop/status, pitch, pan, volume,
      spatialization, position/direction/velocity, cone, doppler/directional-attenuation, min/max
      distance/gain, attenuation, playing-offset and effect-processor surface is generated per class
      from `ext/audio/sound_source.inc`
- [x] **SoundStatus** / **SoundChannel** — `SFML::SoundStatus`, `SFML::SoundChannel`
      (`ext/audio/audio_enums.c`)
- [x] **SoundBuffer** — `SFML::SoundBuffer` (`ext/audio/sound_buffer.c`); loading from file, memory,
      stream and raw samples, saving, sample access and channel map
- [x] **Sound** — `SFML::Sound` (`ext/audio/sound.c`); derives from `SoundSource`
- [x] **SoundStream** — `SFML::SoundStream` (`ext/audio/sound_stream.c`); derives from `SoundSource`,
      subclass and implement `#on_get_data` (and optionally `#on_seek`)
- [x] **SoundRecorder** — `SFML::SoundRecorder` (`ext/audio/sound_recorder.c`); subclass and
      implement `#on_process` (and optionally `#on_start`/`#on_stop`)
- [x] **SoundBufferRecorder** — `SFML::SoundBufferRecorder`
      (`ext/audio/sound_buffer_recorder.c`); derives from `SoundRecorder`
- [x] **Music** — `SFML::Music` (`ext/audio/music.c`); derives from `SoundStream`; file, memory and
      stream sources, loop points
- [x] **EffectProcessor** — `SFML::SoundSource#effect_processor=`; a Ruby proc is dispatched through
      a bounded pool of C thunks (`ext/audio/effect_processor.c`), because `sfEffectProcessor` has
      no `userData` to identify the source
- [x] **Build support** — `ext/ports.rb` builds Ogg 1.3.5, Vorbis 1.3.7 and FLAC 1.4.3 as pinned,
      static, position-independent ports (SFML 3's audio backend is miniaudio, so no OpenAL), and
      enables `SFML_BUILD_AUDIO` / `CSFML_BUILD_AUDIO`

## Network

Socket-based communication and higher-level protocols.

- [x] **IpAddress** — `SFML::IpAddress` (`ext/network/ip_address.c`); string/bytes/integer
      constructors, `NONE`/`ANY`/`LOCAL_HOST`/`BROADCAST`, local and public address lookup
- [x] **Packet** — `SFML::Packet` (`ext/network/packet.c`); raw data plus every typed reader/writer
      (booleans, all integer widths, floats, string)
- [x] **SocketSelector** — `SFML::SocketSelector` (`ext/network/socket_selector.c`); add/remove for
      each socket type, wait and readiness checks
- [x] **TcpSocket** / **TcpListener** — `SFML::TcpSocket`, `SFML::TcpListener`
      (`ext/network/tcp_socket.c`, `ext/network/tcp_listener.c`); connect/accept, blocking control,
      raw and packet sends/receives
- [x] **UdpSocket** — `SFML::UdpSocket` (`ext/network/udp_socket.c`); bind/send/receive and packet
      variants, datagram size and any-port helpers
- [x] **Http** — `SFML::Http`, `SFML::HttpRequest`, `SFML::HttpResponse` (`ext/network/http.c`)
- [x] **Ftp** — `SFML::Ftp`, `SFML::FtpResponse`, `SFML::FtpDirectoryResponse`,
      `SFML::FtpListingResponse` (`ext/network/ftp.c`)
- [x] **SocketStatus** / **HttpMethod** / **HttpStatus** / **FtpStatus** / **FtpTransferMode** —
      enum modules (`ext/network/network_enums.c`)
- [x] **Build support** — `ext/ports.rb` enables `SFML_BUILD_NETWORK` / `CSFML_BUILD_NETWORK`; the
      module needs no external dependency (just `ws2_32` on Windows)

Note that CSFML exposes no `sfSocket` base class, so there is nothing to bind for "Socket base" —
only the concrete TCP/UDP sockets and the selector.

## Pending upstream: SFML 3.1.0

Everything above is the complete CSFML 3.0.0 surface. The gem vendors **SFML 3.0.2 + CSFML 3.0.0**
(`ext/ports.rb`); CSFML's latest release is still **3.0.0**, so the SFML 3.1.0 additions below have
no C entry point to bind yet. They unblock together when a CSFML 3.1 is vendored — the extension
links CSFML and SFML statically, so a CSFML bump carries the matching SFML into every binary gem.

Some of these land in two buckets:

- **Needs a new CSFML binding** — new C entry points the gem would wrap in new `.c` files:
  - **Dns** — new `SFML::Dns` module mirroring `sf::Dns`: `resolve`, `query_ns`, `query_mx`,
    `query_srv`, `query_txt`, `get_public_address`, plus `SFML::Dns::MxRecord` /
    `SFML::Dns::SrvRecord`.
  - **Sftp** — new `SFML::Sftp` client (and its `Result`/`PathResult`/`Attributes`/
    `AttributesResult`/`ListingResult`/`SessionInfo`/`HostKey` types). Upstream positions SFTP as
    the replacement for FTP.
  - **PlaybackDevice** — a `SFML::PlaybackDevice` module for `sf::PlaybackDevice`, notably
    `get_device_sample_rate`.
  - **version()** — runtime `SFML.version` / library-version constant backed by `sf::version()`
    (today `SFML::VERSION` is only the gem's own version).
- **Arrives with the SFML bump, no new binding work** — behavior gained by linking the newer SFML,
  surfacing through entry points already bound:
  - **TLS / HTTPS** in `Http` (SFML 3.1 makes `sf::Http` TLS-capable).
  - **IPv6** in `IpAddress`, `TcpSocket`, `UdpSocket`, `SocketSelector` (SFML 3.1's sockets and
    `IpAddress` understand IPv6).
  - **QOI image format** in `Image` load/save — SFML 3.1 adds QOI support; `sfImage_loadFromFile`
    / `saveToFile` pick it up from the extension.
  - **Shaped text** (SFML 3.1's revamped HarfBuzz-based text engine). SFML's `sf::Text` renders
    complex layouts natively; whether glyph-level shaping (`sf::Text::ShapedGlyph`) is reachable
    depends on what CSFML 3.1 chooses to expose.

C++-only or platform-only 3.1 changes never surface in a bindable way: non-const
`Event::getIf`/`visit` overloads, `Event::visit`, the Android/iOS window and joystick fixes, and
the `sf::Style` namespace (already handled as window-style symbols in `SFML::Window`'s constructor).

## Deliberately unbound

These CSFML entry points have no Ruby surface on purpose. Listed so a coverage diff can be read
without re-deriving the reasoning each time.

| Symbols | Why |
| --- | --- |
| `sfWindow_*` (~26 functions) | A plain `sfWindow` (an OpenGL context with no render-target surface); `SFML::Window` wraps `sfRenderWindow` and `SFML::WindowBase` wraps `sfWindowBase`. |
| `sfRenderWindow_create`, `sfWindowBase_create`, `sfText_getString`/`setString`, `sfRenderWindow_setTitle`, `sfWindowBase_setTitle`, `sfFtpDirectoryResponse_getDirectory` | Superseded by their `*Unicode` counterparts; the narrow forms decode through the C locale and mangle non-ASCII. |
| `sfColor_add`/`subtract`/`modulate`/`fromRGB`/`fromRGBA`/`fromInteger`/`toInteger`, `sfIntRect_contains`/`intersects` | Reimplemented directly in C in `color.c` / `rect.c`; the Ruby methods exist. |
| `sfSprite_getTexture`, `sfText_getFont`, `sf*Shape_getTexture` | The Ruby getters return the cached wrapper object. CSFML returns a non-owning pointer, so re-wrapping it would hand Ruby an object it must not free. |
| `sfShape_getPoint`, `sfShape_getPointCount` | `SFML::Shape` reads these from the Ruby subclass, which is where they are defined. |
| `sfTexture_updateFromWindow` | `Texture#update_from_window` uses `sfTexture_updateFromRenderWindow`, since `SFML::Window` is a render window. |
| `sfFree` | CSFML's allocator hook; nothing in a Ruby binding should call it. |
| `sfGlslVec4_fromsfColor`, `sfGlslIvec4_fromsfColor` | Documentation-only helpers; `Shader#set_color` does the conversion. |

## Rubyesque (Matz-like) layer

The coverage above is the native, 1:1 CSFML surface. `lib/sfml/rubyesque.rb` adds a Rubyesque
(Matz-like) pure-Ruby layer on top of it — predicates (`?`), mutators (`!`), block iterators
(`poll_events!`, `render!`) and scoped resources (`WindowBase.open`,
`SoundBufferRecorder.record!`, `Clock.measure`) — without changing or removing any binding.
The names that shipped before it stay as deprecated aliases that warn and delegate. The README's
"Rubyesque (Matz-like) layer" section is the narrative and `examples/rubyesque/` teaches each area.

| Class / module | Rubyesque additions on top of the native surface |
| --- | --- |
| `WindowBase` | `open?`, `focused?`, `visible?`, `request_focus!`, `poll_events!` (block or Enumerator), scoped `.open` |
| `Window`, `RenderWindow` | the above plus `clear!`, `display!` and the one-call `render!` |
| `Event` | `code` (the `key[:code]` shortcut) and a `?` predicate per kind (`closed?`, `key_pressed?`, `mouse_moved?`, `touch_began?`, ...) |
| `SoundSource` | `playing?`, `paused?`, `stopped?`, shared by Sound, SoundStream and Music |
| `Sound`, `SoundStream`, `Music` | `play!`, `pause!`, `stop!` |
| `SoundBufferRecorder` | scoped `.record!`, returning the captured SoundBuffer |
| `Clock` | `.measure`, timing a block with a throwaway clock |
| `Clipboard` | `content`/`content=`, `has_text?`, `clear!` |
| `Keyboard` | `key_pressed?` |
| `Joystick` | `axis?` |
| `Sensor` | `enable!`, `disable!` |
| `Touch` | `position(finger, relative_to:)` |
| `SFML`, `SFML::Sleep` | `SFML.sleep!` and the namespace-style `SFML::Sleep.sleep!` |


## References

- [SFML 3.1.0 module topics](https://www.sfml-dev.org/documentation/3.1.0/topics.html)
- [SFML 3.1.0 class index](https://www.sfml-dev.org/documentation/3.1.0/annotated.html)
- [SFML 3.1.0 namespace index](https://www.sfml-dev.org/documentation/3.1.0/namespaces.html)
- [SFML 3.1.0 release notes](https://github.com/SFML/SFML/releases/tag/3.1.0) — what the "Pending
  upstream" section is tracking
- [CSFML 3.0.0 headers](https://github.com/SFML/CSFML/tree/3.0.0/include/CSFML) — the actual C API
  surface this gem binds against, vendored locally under `ports/<target>/include/CSFML/`
  once `rake ports` has run (CSFML has no 3.1 release yet)