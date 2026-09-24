---
layout: default
title: Scrolling
parent: Views
grand_parent: "Part IV — Advanced"
nav_order: 4
---

# Recipe: Scrolling

Sometimes the camera is not following anything — the player is *driving* it. Here the arrow keys pan a `View` across a world larger than the window, an auto-scroll loops it back to the left edge, and the wheel zooms. It is the map-viewer side of the same class used for follow cameras.

> **Advanced**

## Goal

A 900×560 window onto a 2400×1600 world with a grid and numbered column markers. The arrow keys pan, `A` toggles auto-scroll (which wraps around when it reaches the far edge), and the mouse wheel zooms. The HUD reads the camera's live centre, size and zoom. Escape quits.

## How the code works

### 1. Load SFML and build the HUD helper

`require 'sfml'` loads the bindings and the three `include` lines bring the subsystem names into scope. One `Font` is loaded from the assets directory, and the `text` helper wraps `Text.new` with a fixed colour and position so every readout is one call.

{% example ruby examples/graphics/views_scrolling/views_scrolling.rb 12-26 %}

### 2. Set the world size and open the window

`WIDTH` and `HEIGHT` are the window while `WORLD_WIDTH` and `WORLD_HEIGHT` are the 2400×1600 world behind it. `Window.new` creates the surface and `frame_rate = 60` caps the loop.

{% example ruby examples/graphics/views_scrolling/views_scrolling.rb 28-34 %}

### 3. Build the grid

A `VertexArray` of `:lines` is filled in 100-pixel steps across and down the world, so there is always something moving under the camera to make panning legible.

{% example ruby examples/graphics/views_scrolling/views_scrolling.rb 36-46 %}

### 4. Column markers and the finish line

Six vertical `RectangleShape` markers are placed every 400px so horizontal motion is obvious, and a bright `finish` bar marks the far edge. They are positioned in world coordinates and drawn through the camera.

{% example ruby examples/graphics/views_scrolling/views_scrolling.rb 48-57 %}

### 5. Create the camera and remember its base size

`View.from_rect` starts the camera window-sized and `center` places it near the top-left corner of the world. `base_size` is kept so the HUD can later derive a zoom ratio, and `auto` tracks whether auto-scroll is on.

{% example ruby examples/graphics/views_scrolling/views_scrolling.rb 59-63 %}

### 6. Poll events: quit, toggle auto-scroll, zoom on the wheel

`'closed'` and `:escape` quit; `:a` flips the `auto` flag. A `'mouse-wheel-scrolled'` event reads `mouse_wheel_scroll[:delta]` and calls `camera.zoom(0.9)` for zoom-in or `1.1` for zoom-out.

{% example ruby examples/graphics/views_scrolling/views_scrolling.rb 65-78 %}

### 7. Pan, auto-scroll and wrap the centre

A per-frame `dx` and `dy` is accumulated from the arrow keys at 320px/s, and when `auto` is on another 160px/s is added. Horizontally the new x wraps to the opposite edge when it passes either margin, so the scroll runs forever; vertically the y is simply clamped.

{% example ruby examples/graphics/views_scrolling/views_scrolling.rb 80-95 %}

### 8. Draw the world, then read the camera back for the HUD

`window.view = camera` draws the grid, markers and finish line in world coordinates. After restoring `default_view`, the HUD prints the live centre and size and derives `zoom = base_size.x / size.x`, so the viewport's own state is visible.

{% example ruby examples/graphics/views_scrolling/views_scrolling.rb 97-113 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `View` | Graphics | The panned camera | `from_rect`, `center=`, `center`, `size`, `zoom` |
| `Window` | Window | Where the view is applied | `view=`, `default_view` |
| `VertexArray` / `Vertex` | Graphics | The world grid | `new`, `primitive=`, `append` |
| `RectangleShape` | Graphics | Column markers and the finish line | `new`, `position=`, `fill_color=` |
| `Keyboard` / `Event` | Window | Panning, auto-scroll and zoom | `key_pressed?`, `mouse_wheel_scroll` |

See the [Graphics API]({% link api/graphics.md %}) for `View`.

## The complete script

The complete program, ready to copy into `examples/graphics/views_scrolling/views_scrolling.rb` and run.

{% example ruby examples/graphics/views_scrolling/views_scrolling.rb %}

{: .note }
> `View#move(offset)` shifts the centre for you, and `View#scissor` clips the view to a target rectangle. The same code underlies map editors and minimaps: one camera for the map, another small view sharing the window.
