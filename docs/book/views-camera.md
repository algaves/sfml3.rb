---
layout: default
title: Camera
parent: Views
grand_parent: "Part IV — Advanced"
nav_order: 1
---

# Recipe: Camera

A camera is a `View` you move to follow something. This is the base case: a world larger than the window, a player moving through it, and a view centred on the player but clamped so the camera stops at the world's edge instead of revealing the void beyond.

> **Advanced**

## Goal

A 900×560 window onto a 1600×1000 world with a grid, four landmarks and a player. The camera tracks the player every frame, clamped to the world bounds. The arrow keys move; escape quits.

## How the code works

### 1. Load SFML and build the HUD helper

`require 'sfml'` loads the bindings and the three `include` lines bring `SF::Window`, `SF::Graphics` and `SF::System` names into scope so classes can be written unqualified. One `Font` is loaded from the example's assets directory, and the `text` helper wraps `Text.new` with a consistent colour and position so every label is one call.

{% example ruby examples/graphics/views_camera/views_camera.rb 13-27 %}

### 2. Set the world size and open the window

`WIDTH` and `HEIGHT` describe the window while `WORLD_WIDTH` and `WORLD_HEIGHT` describe the larger world behind it. `Window.new(VideoMode.new(WIDTH, HEIGHT, 32), title)` creates the OS window and its surface, and `frame_rate = 60` caps the loop so the CPU is not burned at hundreds of frames a second.

{% example ruby examples/graphics/views_camera/views_camera.rb 29-35 %}

### 3. Create the player and the landmarks

The player is a `CircleShape` whose `origin` is set to its radius, so its `position` is its centre. `LANDMARKS` is a frozen array of `[x, y, color]` triples, mapped once into `RectangleShape`s so movement across the world is visible at a glance.

{% example ruby examples/graphics/views_camera/views_camera.rb 37-53 %}

### 4. Build the world grid from one VertexArray

A `VertexArray` of `:lines` is filled by walking `0..WORLD_WIDTH` and `0..WORLD_HEIGHT` in 100-pixel steps and appending a pair of `Vertex` objects per line. Building it once keeps the whole grid a single draw call.

{% example ruby examples/graphics/views_camera/views_camera.rb 55-65 %}

### 5. Create the camera and its clamp helper

`View.from_rect(Rect.new(0, 0, WIDTH, HEIGHT))` makes a camera the size of the window; its `center` decides what it looks at. `clamp_center` takes a position and the view's own `size` and clamps each axis to half a view inside the world, so the camera never shows the void past an edge.

{% example ruby examples/graphics/views_camera/views_camera.rb 67-74 %}

### 6. Poll events for window control

`window.poll_events! { |event| … }` drains the queue in order: `'closed'` is the window's close button, and a `'key-pressed'` carrying `:escape` calls `window.close!`. Handling input first each frame keeps the rest of the loop simple.

{% example ruby examples/graphics/views_camera/views_camera.rb 76-84 %}

### 7. Move the player and centre the camera

`Keyboard.key_pressed?` reads the held arrow keys and `player.move([dx, dy])` shifts the player by a fixed speed; the new position is clamped to the world so it stops at the walls. `camera.center = clamp_center(player.position, camera.size)` is what makes the camera follow, using the live view size so the clamp survives a zoom.

{% example ruby examples/graphics/views_camera/views_camera.rb 86-95 %}

### 8. Draw the world through the camera, then the HUD in pixels

`window.clear!` erases the frame, `window.view = camera` makes the grid, landmarks and player project through the camera, and each is queued with `window.draw`. Restoring `window.view = window.default_view` returns to pixel coordinates so the text readout stays fixed, and `display!` presents the finished frame.

{% example ruby examples/graphics/views_camera/views_camera.rb 97-111 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `View` | Graphics | The camera | `from_rect`, `center=`, `center`, `size` |
| `Window` | Window | Where a view is applied | `view=`, `default_view` |
| `Rect` | Graphics | A view's world rectangle | `new` |
| `CircleShape` / `RectangleShape` | Graphics | Player and landmarks | `new`, `origin=`, `position=`, `fill_color=` |
| `VertexArray` / `Vertex` | Graphics | The world grid | `new`, `primitive=`, `append` |
| `Keyboard` / `Event` | Window | Movement and quit | `key_pressed?`, `poll_events!` |

See the [Graphics API]({% link api/graphics.md %}) for `View`, and the [Custom Rendering]({% link learn/custom-rendering.md %}) tutorial for the drawing path.

## The complete script

The complete program, ready to copy into `examples/graphics/views_camera/views_camera.rb` and run.

{% example ruby examples/graphics/views_camera/views_camera.rb %}

{: .note }
> `clamp_center` uses the view's own `size`, so it keeps working if you `zoom` the camera: a zoomed-out camera needs a larger margin. See [Scrolling]({% link book/views-scrolling.md %}) for a camera you move yourself.
