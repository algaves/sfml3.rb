---
layout: default
title: Split-Screen
parent: Views
grand_parent: "Part IV — Advanced"
nav_order: 2
---

# Recipe: Split-Screen

One window can host several cameras at once. Each `View` has a **viewport** — a normalised rectangle saying where on the target its image lands — so two views with half-window viewports give a genuine split screen: the same world, rendered twice, with independent positions and zooms.

> **Advanced**

## Goal

A 900×560 window split down the middle. The left camera follows the player; the right camera stays a fixed, zoomed-out overview of the whole world. The arrow keys move; escape quits.

## How the code works

### 1. Load SFML and build the HUD helper

`require 'sfml'` loads the bindings and the three `include` lines bring the subsystem names into scope. One `Font` is loaded from the assets directory, and the `text` helper wraps `Text.new` with a fixed colour and position so the overlay labels are one call each.

{% example ruby examples/graphics/views_split/views_split.rb 13-27 %}

### 2. Set constants, open the window and create the player

`WIDTH`, `HEIGHT`, `HALF` and the world sizes are fixed up front. `Window.new` creates the surface and `frame_rate = 60` caps the loop. The player is a `CircleShape` with its `origin` at its centre, placed in the middle of the world.

{% example ruby examples/graphics/views_split/views_split.rb 29-41 %}

### 3. Landmarks and a shared world grid

Four `[x, y, color]` landmarks are mapped into `RectangleShape`s. The grid is a `VertexArray` of `:lines` filled in 100-pixel steps across and down the world; it is built once and reused by both views.

{% example ruby examples/graphics/views_split/views_split.rb 43-62 %}

### 4. Make two views and give them half-window viewports

`View.from_rect` sizes each camera to `HALF` × `HEIGHT` so its aspect matches its viewport. `left.viewport = [0.0, 0.0, 0.5, 1.0]` claims the left half and `right.viewport = [0.5, 0.0, 0.5, 1.0]` the right; viewport coordinates are normalised, so they survive a window resize.

{% example ruby examples/graphics/views_split/views_split.rb 64-68 %}

### 5. Configure the overview camera and the clamp

The right camera is pinned to the world centre and `zoom(3.6)` pulls it back until the whole 1600×1000 map fits. `clamp_center` is the same half-view margin the follow camera uses.

{% example ruby examples/graphics/views_split/views_split.rb 70-77 %}

### 6. Poll events and move the player

`poll_events!` handles `'closed'` and `:escape` for quitting. The arrow keys read with `Keyboard.key_pressed?` then move the player a fixed amount per frame, and its position is clamped to the world so it stops at the walls.

{% example ruby examples/graphics/views_split/views_split.rb 79-95 %}

### 7. Draw the scene once per view

After `window.clear!`, the loop walks `[left, right]`, sets `window.view` to each and draws the grid, landmarks and player again. Every draw is projected through whichever view is active, so the same objects appear in both halves.

{% example ruby examples/graphics/views_split/views_split.rb 99-107 %}

### 8. Draw the divider and labels in pixel coordinates

`window.view = window.default_view` restores pixel coordinates. The divider is a thin `RectangleShape` at the seam and the three labels come from the `text` helper; none of the chrome is transformed by either camera.

{% example ruby examples/graphics/views_split/views_split.rb 109-118 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `View` | Graphics | One camera per viewport | `from_rect`, `viewport=`, `center=`, `zoom` |
| `Window` | Window | Hosts both views | `view=`, `default_view` |
| `RectangleShape` | Graphics | Divider and landmarks | `new`, `position=`, `fill_color=` |
| `CircleShape` | Graphics | The player | `new`, `origin=`, `position=` |
| `Rect` | Graphics | A view's world rectangle | `new` |
| `Keyboard` / `Event` | Window | Movement and quit | `key_pressed?`, `poll_events!` |

See the [Graphics API]({% link api/graphics.md %}) for `View`.

## The complete script

The complete program, ready to copy into `examples/graphics/views_split/views_split.rb` and run.

{% example ruby examples/graphics/views_split/views_split.rb %}

{: .note }
> Because each view owns a viewport, a split screen costs only the extra draws — there is no second window or texture. `View#scissor` clips a view to a target rectangle when the viewport alone should not show it all; for a two-player variant, give each view its own input and target.
