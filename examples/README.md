# Examples

Runnable demos for the binding, grouped by what they show. None of them is part
of the test suite.

* `hello_shapes.rb` and `bouncing_shapes.rb` (this directory) are the minimal
  walkthrough: the five building blocks -- Window, Events, Transformable,
  Drawable objects, primitive shapes -- used once, then reinterpreted as an
  interactive app.
* `subsystems/` has one example per window/audio/graphics/network surface.
* `games/` has eleven small, complete games built from the same components,
  plus the `menu_demo.rb` walkthrough of the shared menu (`examples/menu.rb`).

They need a real display; SFML aborts rather than raises when there is none.
Headless machines can run any of them under `xvfb-run -a`.

## Running

Run from the repository root. `bundle exec` keeps the checkout's freshly built
extension from being shadowed by an installed gem; with the gem installed
instead, plain `ruby -Ilib examples/subsystems/mouse.rb` also works.

```
bundle exec ruby -Ilib examples/subsystems/mouse.rb
bundle exec ruby -Ilib examples/games/snake.rb
```

| Component          | What it does                          | Key API (see `sig/`)                                                                  |
| ------------------ | ------------------------------------- | ------------------------------------------------------------------------------------- |
| Window             | Owns the OS window and render loop    | `Window.new(VideoMode.new(w, h, bpp), title)`, `frame_rate=`, `clear!`, `display!`, `open?`, `close!`, `render!` |
| Events             | Input and window notifications        | `Event.new`, `poll_events! { \|event\| }`, `event.type`, `event.closed?`, `event.key_pressed?`, `event.code`, `event.mouse_button`, `event.mouse_move` |
| Transformable      | Position, rotation, scale, origin     | `position=`, `move`, `rotate`, `scale=`, `origin=` (a mixin included by every drawable) |
| Drawable objects   | Anything `window.draw` accepts        | `Drawable` mixin; `Shape`/`Sprite`/`Text`/`VertexArray` include it                    |
| Primitive shapes   | The built-in filled shapes            | `CircleShape`, `RectangleShape`, `ConvexShape#set_point`, `fill_color=`, `outline_*`    |

A new example built on these components only needs to fill in the loop between
`clear!` and `display!`:

```ruby
window = Window.new(VideoMode.new(640, 480, 32), 'title')

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed' then window.close!
    when 'key-pressed' then window.close! if event.code == :escape
    end
  end

  # move / rotate / scale the transformables...
  window.clear!([24, 24, 34, 255]) # [r, g, b, a], 0-255
  # window.draw(shape)...
  window.display!
end
```

Every interactive example reads `SFML_EXAMPLE_FRAMES`: set it to close the
window after that many frames instead of waiting for a human. Useful for smoke
tests:

```
SFML_EXAMPLE_FRAMES=120 xvfb-run -a bundle exec ruby -Ilib examples/games/snake.rb
```

The console-only examples (`dns.rb`; `vulkan.rb` when there is no driver)
ignore the variable and exit on their own.

## Subsystems

| File | SFML module | What it shows |
| --- | --- | --- |
| `subsystems/listener.rb` | `Listener` | Position/direction/up-vector/volume and a spatialized looping `Sound` with a cone, moved with the arrow keys |
| `subsystems/audio_devices.rb` | `SoundRecorder` | Audio devices: `available_devices`/`default_device`/`device=`, plus a 3-second `SoundBufferRecorder` capture played back through a `Sound` |
| `subsystems/glsl.rb` | `Shader` | `Shader.from_memory` on a full-screen rectangle, with `u_time`/`u_resolution`/`u_mouse` uniforms updated per frame |
| `subsystems/dns.rb` | `IpAddress` | Local/public address lookup, parsing round-trips, and hostname resolution through `TcpSocket#connect` |
| `subsystems/literals.rb` | `Color`/`Vector2`/`Vector3`/`Rect`/`Time` | The Ruby literal forms the API accepts (`[r,g,b,a]`, `[x,y]`, `[l,t,w,h]`, ...) and their round-trips |
| `subsystems/clipboard.rb` | `Clipboard` | Copy/paste via `string`/`unicode_string`, including non-ASCII round-trips |
| `subsystems/joystick.rb` | `Joystick` | Connected pads, identification, live axes and buttons; R rescans |
| `subsystems/keyboard.rb` | `Keyboard` | Live key state plus `localize`/`delocalize`/`description` and the virtual-keyboard toggle |
| `subsystems/mouse.rb` | `Mouse` | Window-relative and desktop position, per-button state, `set_position` |
| `subsystems/sensor.rb` | `Sensor` | Availability, enable and value for every sensor type, with an "unavailable" panel |
| `subsystems/touch.rb` | `Touch` | Real-time multitouch fingers and the touch events |
| `subsystems/vulkan.rb` | `Vulkan` | Loader availability, required instance extensions, entry-point lookup, and (where a driver exists) a real VkInstance + `create_vulkan_surface` |
| `subsystems/style.rb` | `Window` | Every window style/state combination, rebuilt live, plus min/max size |
| `subsystems/graphics_assets.rb` | `Image`/`Texture`/`Sprite`/`RenderTexture` | A sheet loaded from disk and animated through `Sprite#texture_rect`, a `Texture` built from `Image.from_pixels`, pixel edits, and a live `RenderTexture` drawn back through its texture |

### Topics with no bound class

Three names in SFML's docs have nothing to bind in this gem. The closest bound
surface is used instead, as noted in the table:

* **PlaybackDevice** -- SFML has no such class. Playback lives on `Sound` /
  `Music` (see `listener.rb` and the games), and device enumeration through
  `SoundRecorder.available_devices` is in `audio_devices.rb`.
* **Dns** -- SFML 3.1 added a `Dns` class (MX/SRV records), but this gem vendors
  CSFML 3.0.0, which predates it (the same reason `Sftp` is out of scope).
  `dns.rb` uses `IpAddress` resolution instead.
* **Literals** -- SFML's C++ user-defined literals (`"text"_s`) do not exist in
  C; CSFML takes plain C strings. `literals.rb` shows the Ruby-literal forms the
  binding does accept.

## Games

Every game uses the vendored font for the HUD and the vendored WAVs for sound
effects. Escape quits; R restarts. Tron and Racing start with two players on one
keyboard (Tron: WASD vs arrows; Racing: arrows vs WASD) and `M` swaps the second
player for the computer. `menu_demo.rb` is the GUI showcase for
`examples/menu.rb`.

| File | Game | What it shows |
| --- | --- | --- |
| `games/snake.rb` | Snake | Grid state, a fixed-timestep `Clock`, growing collections, death/restart |
| `games/breakout.rb` | Breakout | Paddle/ball physics, AABB brick collision, lives, win/lose |
| `games/asteroids.rb` | Asteroids | Rotate/thrust, edge wrapping, splitting asteroids, bullets, respawn invulnerability |
| `games/platformer.rb` | Platformer | Tile collision, gravity and jumping, a scrolling `View`, pickups and a goal |
| `games/tron.rb` | Tron | Trail grid, collision, round scoring; two-player WASD vs arrows, `M` swaps P2 for a flood-fill AI |
| `games/flappy_bird.rb` | Flappy Bird | Gravity/flap, scrolling pipe sprites, a scrolling ground texture, score/best |
| `games/doodle_jump.rb` | Doodle Jump | Auto-bounce, moving/breakable platforms, springs, an upward `View` camera |
| `games/xonix.rb` | Xonix | Territory capture by drawing lines, flood-fill scoring, bouncing enemies, lives |
| `games/tetris.rb` | Tetris | 7-bag tetrominoes, rotation, ghost drop, hold/next, line clears, level speed-up |
| `games/racing_car.rb` | Racing | Generated track, car sprites, checkpoint laps and best-lap timing; two-player, `M` swaps P2 for a waypoint AI |
| `games/chess.rb` | Chess | Full rules hot-seat: legal-move filtering, check/checkmate/stalemate, castling, en passant, promotion, undo |
| `games/menu_demo.rb` | GUI demo | The `ExampleSupport::Gui` toolkit: a draggable panel with buttons, checkboxes, a radio group and sliders driving a live preview |

## Assets and shared helpers

* `support.rb` caches the font, the WAVs and the sprite sheet, builds HUD `Text`,
  generates `Image`/`Texture` content from a block, hands out `Sprite`s from the
  sheet, supplies a two-point `VertexArray` line helper, and implements the
  `auto_close?` testing hook.
* `menu.rb` is a small immediate-mode GUI (`ExampleSupport::Gui`): an `Input`
  with edge-detected clicks plus panel, button, checkbox, slider, radio and
  gauge helpers. `games/menu_demo.rb` builds a control panel with it.
* `assets/LiberationSans-Regular.ttf` -- Liberation Sans, licensed under the
  SIL Open Font License 1.1; the license text is beside it in
  `assets/LICENSE-LiberationSans.txt`.
* `assets/*.wav` -- short tones generated with SoX (`sox -n beep.wav synth ...`),
  so no third-party audio is bundled.
* `assets/sprites.png` -- the only bundled image, a 128x128 sheet of 16x16 cells
  (bird frames, pipe, platform, gem, car, doodler, tetromino cells, coin, bricks,
  grass, enemy, star, block, spring, arrow). It is generated from the binding
  itself, not drawn by hand:

  ```
  bundle exec ruby -Ilib script/generate_example_sprites.rb
  ```

  The cell coordinates are mirrored in `ExampleSupport::SPRITES`; keep the two in
  sync. Games that need something else build it at runtime with
  `ExampleSupport.procedural_texture`.
