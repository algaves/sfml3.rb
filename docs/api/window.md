---
layout: default
title: Window
parent: API Reference
nav_order: 3
---

# Window API

OS windows, OpenGL contexts, video modes, and events. Canonical namespace is `SF::Window`.

```ruby
require 'sfml'
```

Hierarchy: `SF::Graphics::RenderWindow < SF::Window::Window < SF::Window::WindowBase`. `WindowBase` wraps `sfWindowBase` (OS window + event queue, no GL context); `Window` wraps `sfRenderWindow` (adds a context and stays renderable, so `Window.new(...).clear` keeps working); `RenderWindow` adds the `RenderTarget` drawing surface (see [Graphics]({% link api/graphics.md %})).

## Contents

- [WindowBase / Window](#windowbase--window)
- [VideoMode / Style / State](#videomode-style--state)
- [Event](#event)
- [Context / ContextSettings](#context--contextsettings)
- [Cursor](#cursor)
- [Vulkan](#vulkan)

---

## WindowBase & Window

### `SF::Window::WindowBase`

OS window + event queue, no OpenGL context. Shared methods with `Window` are generated from `ext/window/window_base.inc`.

```ruby
mode = SF::Window::VideoMode[800, 600, 32]
SF::Window::WindowBase.open(mode, 'Plain') do |window|
  while window.open?
    window.poll_events! { |e| window.close! if e.closed? }
  end
end
```

| Method | Description |
|---|---|
| `WindowBase.new(mode, title, style = :default, state = :windowed)` | Create |
| `WindowBase.from_handle(handle)` | Wrap a native handle |
| `WindowBase.open(...) { \|w\| }` | Scoped: closes automatically; returns window without block |
| `open! { }` | Block loop: yields while open, closes after |
| `poll_event!(event)` / `poll_event! { \|e\| }` | Drain one event (bool / block form) |
| `wait_event!(event)` / `wait_event! { \|e\| }` | Block for one event |
| `poll_events! { \|e\| }` / enumerator | Yield every pending event |
| `open?` (`is_open?` deprecated) | Still open? |
| `close!` | Close |
| `size` / `size=` / `position` / `position=` | Geometry (`Vector2`) |
| `title=` | Retitle |
| `focused?` (`focus?` deprecated) / `visible?` | State predicates |
| `request_focus!` | Ask WM for focus |
| `mouse_cursor=` / `cursor=` | Attach a `Cursor` |
| `joystick_threshold=` | Deadzone for `Joystick` axes |
| `create_vulkan_surface(instance, allocator = nil)` | Vulkan surface handle (nullable) |

{: .note }
> `WindowBase` has no `clear` / `display` / `draw` — use `Window` / `RenderWindow` for rendering. Never `dup`/`clone` (raises `TypeError`). Reuse one `Event.new` in poll loops.

### `SF::Window::Window < WindowBase`

Window + OpenGL context (+ `RenderTarget` methods, so it stays directly renderable).

```ruby
settings = SF::Window::ContextSettings.new(24, 8, 4)
window = SF::Window::Window.new(mode, 'GL', SF::Window::Style::DEFAULT, SF::Window::State::WINDOWED, settings)
window.framerate_limit = 60
# window.vertical_sync_enabled = true # pick ONE: framerate limit OR vsync
window.clear([51, 76, 102, 255])
window.display!
```

| Method | Description |
|---|---|
| `Window.new(mode, title, style, state, settings)` | With GL context |
| `Window.from_handle(handle, settings = nil)` | Wrap native handle |
| `clear(color)` / `clear!`, `display` / `display!` | Present loop |
| `draw(drawable, state = nil)` | Draw directly |
| `render! { \|t\| }` | Rubyesque: clear → yield → display |
| `settings` | Granted `ContextSettings` (read back — request ≠ grant) |
| `view` / `default_view` / `view=` | Camera |
| `framerate_limit=` (`frame_rate=` alias) / `vertical_sync_enabled=` | Throttling (enable one, not both) |
| `active=` | Context activation for threaded loading |
| `native_handle` | OS handle for embedding / FFI |

{: .warning }
> `framerate_limit` + VSync conflict — enable one. `active=` must be managed when sharing contexts across threads.

---

## VideoMode, Style & State

### `SF::Window::VideoMode`

Width / height / bits-per-pixel display mode value object.

```ruby
mode = SF::Window::VideoMode.new(800, 600, 32)
mode = SF::Window::VideoMode[800, 600] # bits defaults to 32
puts SF::Window::VideoMode.desktop_mode.inspect
puts SF::Window::VideoMode.fullscreen_modes.size
window = SF::Window::Window.new(mode, 'Go') if mode.valid?
```

| Method | Description |
|---|---|
| `width` / `height` / `bits` | Fields |
| `size` | As `Vector2` |
| `valid?` / `available?` | Usable? (`valid?` = valid for fullscreen) |
| `==` | Comparison |
| `VideoMode.desktop_mode` | Current desktop mode |
| `VideoMode.fullscreen_modes` | Sorted supported list |

{: .note }
> Always validate custom modes with `valid?` before opening a fullscreen window.

### `SF::Window::Style` — decoration flags

Integer flag namespace mirroring `sfStyle`; combine with `|`:

```ruby
window = SF::Window::Window.new(mode, 'Borderless', SF::Window::Style::NONE)
window = SF::Window::Window.new(mode, 'Fixed', :titlebar) # Symbol / Array<Symbol> / Integer all accepted
```

`NONE TITLEBAR RESIZE CLOSE DEFAULT` (`:default` == `TITLEBAR|RESIZE|CLOSE`). Pass `Style::NONE` (or `:none`) for borderless.

### `SF::Window::State` — windowed vs fullscreen

`WINDOWED FULLSCREEN` (`:windowed` / `:fullscreen` in constructors — the 4th arg, distinct from `Style`). Fullscreen needs a `VideoMode#valid?` mode.

---

## Event

### `SF::Window::Event`

Single polled OS event (reused buffer object). Types are strings (`'closed'`, `'resized'`, `'key-pressed'`, …); every payload is a Hash; every kind has a Rubyesque predicate plus the `code` shortcut.

```ruby
event = SF::Window::Event.new
while window.poll_event!(event)
  case event.type
  when 'closed' then window.close!
  when 'key-pressed' then puts event.code.inspect # event.key[:code] shortcut
  when 'resized' then puts event.size.inspect
  end
end

# Rubyesque block form (preferred):
window.poll_events! do |e|
  window.close! if e.closed?
  puts "key: #{e.code}" if e.key_pressed?
  puts e.mouse_move.inspect if e.mouse_moved?
end
```

| Accessor | Valid when | Shape |
|---|---|---|
| `type` | always | `String` |
| `code` | key events | `event.key[:code]` shortcut (`:escape`, `:Space`, …) |
| `key` | `key_pressed?` / `key_released?` | `{ code:, scancode:, alt:, control:, shift:, system: }` |
| `text` | `text_entered?` | `{ unicode: }` |
| `size` | `resized?` | `{ width:, height: }` |
| `mouse_move` / `mouse_move_raw` | `mouse_moved?` / `mouse_moved_raw?` | `{ x:, y: }` (+ deltas for raw) |
| `mouse_button` | `mouse_button_pressed?` / `released?` | `{ button:, x:, y: }` |
| `mouse_wheel_scroll` | `mouse_wheel_scrolled?` | `{ wheel:, delta:, x:, y: }` |
| `joystick_move` / `joystick_button` / `joystick_connect` | joystick events | `{ joystick_id:, axis:, position: }` etc. |
| `touch` | `touch_began?` / `moved?` / `ended?` | `{ finger:, x:, y: }` |
| `sensor` | `sensor_changed?` | `{ type:, x:, y:, z: }` |

Predicates: `closed?`, `resized?`, `focus_gained?`, `focus_lost?`, `mouse_entered?`, `mouse_left?`, `key_pressed?`, `key_released?`, `text_entered?`, `mouse_moved?`, `mouse_moved_raw?`, `mouse_button_pressed?`, `mouse_button_released?`, `mouse_wheel_scrolled?`, `joystick_moved?`, `joystick_button_pressed?`, `joystick_button_released?`, `joystick_connected?`, `joystick_disconnected?`, `touch_began?`, `touch_moved?`, `touch_ended?`, `sensor_changed?`.

{: .warning }
> Payload accessors are only valid for the matching type — reading `size` on a key event is meaningless. `wait_event!` blocks; prefer the `poll_events!` loop.

---

## Context & ContextSettings

### `SF::Window::ContextSettings`

Requested GL context attributes value object.

```ruby
settings = SF::Window::ContextSettings.new(24, 8, 4) # depth, stencil, antialiasing
settings.major_version = 3
settings.minor_version = 3
settings.srgb_capable = true
window = SF::Window::Window.new(mode, 'AA', SF::Window::Style::DEFAULT, :windowed, settings)
puts window.settings.antialiasing_level # granted value — may differ
```

Fields: `depth_bits`, `stencil_bits`, `antialiasing_level`, `major_version` / `minor_version`, `attribute_flags` (`Array<Symbol>`), `srgb_capable?` (+ setters).

{: .note }
> Request ≠ grant — read back `Window#settings`. AA / depth cost performance; request minimally.

### `SF::Window::Context`

Headless OpenGL context for threaded resource loading.

```ruby
ctx = SF::Window::Context.new
ctx.active = true
puts SF::Window::Context.extension_available?('GL_EXT_texture_filter_anisotropic')
puts SF::Window::Context.active_context_id
```

Only one context active per thread. `Context.function(name)` returns a raw pointer (`Integer`) — null means a missing extension.

---

## Cursor

### `SF::Window::Cursor`

Mouse cursor image (no `.new` — factory constructors only, then attach via `WindowBase#mouse_cursor=`).

```ruby
cursor = SF::Window::Cursor.from_system(:hand)
# or from RGBA pixels:
cursor = SF::Window::Cursor.from_pixels(pixels_string, [32, 32], [16, 16])
window.mouse_cursor = cursor
window.mouse_cursor = nil # restore default
```

- `Cursor.from_system(type)` — `:arrow`, `:hand`, `:text`, `:cross`, `:size_horizontal`, … (platform set)
- `Cursor.from_pixels(pixels, size, hotspot)` — `pixels` RGBA string, `size`/`hotspot` as `Vector2`/Array.

{: .note }
> Keep a Ruby reference alive while attached; assign `nil` to restore the default.

---

## Vulkan

### `SF::Window::Vulkan` (module)

Vulkan availability helpers. No instances.

```ruby
if SF::Window::Vulkan.available?(true)
  exts = SF::Window::Vulkan.graphics_required_instance_extensions
  # create VkInstance with exts, then:
  surface = window.create_vulkan_surface(vk_instance_ptr)
end
```

| Method | Description |
|---|---|
| `Vulkan.available?(require_graphics = false)` | Runtime support? |
| `Vulkan.function(name)` | Raw Vulkan function pointer (`Integer`) |
| `Vulkan.graphics_required_instance_extensions` | `Array<String>` — query **before** `VkInstance` creation |

Surface creation is `WindowBase#create_vulkan_surface(instance, allocator)` → nullable handle.

---

## See also

- [Graphics]({% link api/graphics.md %}) — `RenderWindow`, drawing, views
- [Input]({% link api/input.md %}) — Keyboard, Mouse, Joystick, Touch, Sensor, Clipboard
- [Window example]({% link book/window-classes.md %}) — `WindowBase` vs `Window` vs `RenderWindow`
- [Basic Game Loop]({% link learn/basic-game-loop.md %}) — window lifecycle in practice
- RBS under `sig/window/`, YARD comments under `ext/window/`
