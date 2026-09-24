---
layout: default
title: Shader Wave Effect
parent: "Shaders & Vertex Arrays"
grand_parent: "Part IV — Advanced"
nav_order: 1
---

# Recipe: Shader Wave Effect

GLSL distortion driven by a `u_time` uniform — `Shader.from_memory` with both a vertex and a fragment program, guarded by `Shader.available?`.

> **Advanced**

## Goal

A 640×480 window filled by a full-screen rectangle painted by a fragment shader: an animated cosine palette with a soft spotlight that follows the mouse. On drivers without shader support it prints a message instead. Escape quits.

## How the code works

### 1. Load SFML and build the HUD helper

`require 'sfml'` loads the bindings and the three `include` lines bring the subsystem names into scope. One `Font` is loaded from the assets directory, and the `text` helper wraps `Text.new` with a fixed colour and position so both the overlay and the fallback message are one call each.

{% example ruby examples/subsystems/glsl/glsl.rb 15-29 %}

### 2. The vertex shader source

`VERTEX` is a GLSL heredoc that forwards the built-in model-view-projection matrix and vertex colour to the fragment stage. It is the minimum a vertex program must do to place geometry and pass a colour on.

{% example ruby examples/subsystems/glsl/glsl.rb 31-37 %}

### 3. Fragment shader uniforms and coordinates

`FRAGMENT` declares `u_time`, `u_resolution` and `u_mouse`, then normalises `gl_FragCoord` into `uv` in 0..1, recentres it into `p` in -1..1 and applies the aspect ratio. The mouse is put into the same space as `m`, so distances can be compared directly.

{% example ruby examples/subsystems/glsl/glsl.rb 39-50 %}

### 4. The cosine palette and mouse spotlight

`palette` is a cosine of `u_time` and `uv`, giving a smoothly cycling colour. `smoothstep(0.55, 0.0, length(p - m))` measures the distance from the mouse to produce a soft circular spotlight, which is added to the palette before `gl_FragColor` is written.

{% example ruby examples/subsystems/glsl/glsl.rb 52-56 %}

### 5. Compile the shader and build the render state

`Shader.available?` is checked before `Shader.from_memory(VERTEX, FRAGMENT)` compiles and links the pair, leaving `shader` as `nil` on drivers without support. A `RenderState` carries that shader into a draw, and the `Clock` and full-window `canvas` are created once.

{% example ruby examples/subsystems/glsl/glsl.rb 58-65 %}

### 6. Poll events and clear the frame

`poll_events!` drains the queue: `'closed'` and a `:escape` key press close the window. `window.clear!` then erases the previous frame before the shader or fallback draws over it.

{% example ruby examples/subsystems/glsl/glsl.rb 67-77 %}

### 7. Feed the uniforms and draw through the state

Each frame `set_float('u_time', …)`, `set_vec2('u_resolution', …)` and `set_vec2('u_mouse', …)` refresh the three uniforms from the clock, the window size and `Mouse.position(window)`. `window.draw(canvas, state)` then renders the rectangle with the shader applied.

{% example ruby examples/subsystems/glsl/glsl.rb 79-85 %}

### 8. Fall back when shaders are unavailable

When `shader` is `nil` the loop draws a `Text` message quoting `Shader.available?` and `Shader.geometry_available?` instead of crashing. `display!` presents whichever branch ran.

{% example ruby examples/subsystems/glsl/glsl.rb 86-94 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Shader` | Graphics | Compiles and owns the GLSL programs | `available?`, `from_memory`, `set_float`, `set_vec2`, `geometry_available?` |
| `RenderState` | Graphics | Carries the shader into one draw | `new`, `shader=` |
| `RectangleShape` | Graphics | The full-screen canvas | `new`, `size` |
| `Mouse` | Window | Mouse position for `u_mouse` | `position` |
| `Clock` | System | Supplies `u_time` | `new`, `elapsed_time` |
| `Text` / `Font` | Graphics | Overlay and fallback message | `Text.new`, `fill_color=`, `position=` |
| `Window` | Window | Render surface | `new`, `size`, `poll_events!`, `draw`, `display!` |

See the [Graphics API]({% link api/graphics.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/subsystems/glsl/glsl.rb` and run.

{% example ruby examples/subsystems/glsl/glsl.rb %}

{: .note }
> `set_float` and `set_vec2` update uniforms each frame; `Shader.available?` guards compilation. On systems without shader support the example prints a message instead of rendering.
