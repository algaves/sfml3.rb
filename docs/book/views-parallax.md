---
layout: default
title: Parallax
parent: Views
grand_parent: "Part IV — Advanced"
nav_order: 3
---

# Recipe: Parallax

Parallax is the illusion of depth you get when distant things move more slowly than near ones. With views it is almost free: keep the near camera on the player, then put a second camera's centre at half of the near one's. Draw the background through the slow camera and the foreground through the fast one.

> **Advanced**

## Goal

A 900×560 window with two layers: a grid and a field of stars that scroll slowly behind, and solid landmarks plus a player that move at full speed. The HUD prints both camera centres to show the ratio. The arrow keys move; escape quits.

## How the code works

### 1. Load SFML and build the HUD helper

`require 'sfml'` loads the bindings and the three `include` lines bring the subsystem names into scope. One `Font` is loaded from the assets directory, and the `text` helper wraps `Text.new` with a fixed colour and position so every readout is one call.

{% example ruby examples/graphics/views_parallax/views_parallax.rb 13-27 %}

### 2. Set constants, open the window and create the player

`WIDTH`, `HEIGHT` and the world sizes are fixed up front, then `Window.new` creates the surface and caps it at 60 frames a second. The player is a `CircleShape` with its `origin` at its centre, placed in the middle of the world.

{% example ruby examples/graphics/views_parallax/views_parallax.rb 28-39 %}

### 3. Build the far layer: the grid and a star field

The grid is a `VertexArray` of dim `:lines`; `stars` is a second array with `primitive = :points`. `srand(7)` makes the 300 random star positions reproducible, and each is a single cool-grey `Vertex`.

{% example ruby examples/graphics/views_parallax/views_parallax.rb 42-58 %}

### 4. Build the near layer: solid landmarks

Four `[x, y, color]` landmarks are mapped into `RectangleShape`s. They are solid and bright so the near layer reads clearly against the dim, slowly moving background.

{% example ruby examples/graphics/views_parallax/views_parallax.rb 61-69 %}

### 5. Create the two cameras and the clamp

`near` and `far` are both window-sized views, so only their centres will differ. `clamp_center` keeps the near view at least half a view inside the world, exactly as a normal follow camera would.

{% example ruby examples/graphics/views_parallax/views_parallax.rb 71-78 %}

### 6. Poll events and move the player

`poll_events!` handles `'closed'` and `:escape` for quitting. The held arrow keys are read with `Keyboard.key_pressed?` and move the player a fixed amount per frame, clamped to the world so it stops at the walls.

{% example ruby examples/graphics/views_parallax/views_parallax.rb 80-96 %}

### 7. Halve the far centre, then draw far before near

`near.center` follows the player with the clamp, and `far.center = [center.x * 0.5, center.y * 0.5]` makes the far view travel half the distance for the same movement — the whole parallax effect. The grid and stars are drawn through `far` first, then the landmarks and player through `near`, so the near layer paints over the far one.

{% example ruby examples/graphics/views_parallax/views_parallax.rb 98-110 %}

### 8. Restore the pixel view for the HUD

`window.view = window.default_view` returns to pixel coordinates, so the readout is unaffected by either camera, and `display!` presents the frame. It prints both centres side by side, showing the far one at half speed.

{% example ruby examples/graphics/views_parallax/views_parallax.rb 112-119 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `View` | Graphics | Near and far cameras | `from_rect`, `center=`, `center` |
| `Window` | Window | Applies each view in turn | `view=`, `default_view` |
| `VertexArray` / `Vertex` | Graphics | Grid and star fields as `:lines` / `:points` | `new`, `primitive=`, `append` |
| `RectangleShape` / `CircleShape` | Graphics | Landmarks and player | `new`, `position=`, `fill_color=` |
| `Keyboard` / `Event` | Window | Movement and quit | `key_pressed?`, `poll_events!` |

See the [Graphics API]({% link api/graphics.md %}) for `View` and `VertexArray`.

## The complete script

The complete program, ready to copy into `examples/graphics/views_parallax/views_parallax.rb` and run.

{% example ruby examples/graphics/views_parallax/views_parallax.rb %}

{: .note }
> The `0.5` factor is the depth: `0.25` pushes a layer even farther back, `0.0` pins it to the screen entirely. A real side-scroller usually stacks three or four cameras this way, one per layer.
