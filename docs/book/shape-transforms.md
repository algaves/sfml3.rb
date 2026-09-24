---
layout: default
title: Transformable
parent: "Part I — Essential"
grand_parent: Book
nav_order: 4
---

# Recipe: Transformable

The `Transformable` mixin, which every drawable includes: `position`, `rotation`, `scale` and `origin`. `R` switches the rectangle between corner-pivot and centre-pivot, the arrow keys move it, the wheel scales it, and it spins while space is off. The yellow outline is its transformed `global_bounds`, the red dot is the origin's pivot, and the bottom row draws the same shape through composed `Transform`s in a `RenderState`.

> **Essential**

## Goal

An 800×600 window with a rectangle you can move (arrow keys), scale (wheel), spin (space), and re-pivot (`R`). Its transformed bounds and origin are drawn on top, and a row of three wedges at the bottom is positioned entirely by composed `Transform` matrices rather than `Transformable`.

## How the code works

### 1. Load SFML and the shared text helper

The `include` lines expose the window, graphics and clock namespaces, and the `text` helper keeps the HUD creation to a single call.

{% example ruby examples/graphics/shapes_transforms/shapes_transforms.rb 15-29 %}

### 2. Open the window and build the box

`RectangleShape.new([200, 120])` is positioned and styled; the default `origin` `[0, 0]` is the top-left corner, so rotations pivot there until `R` changes it.

{% example ruby examples/graphics/shapes_transforms/shapes_transforms.rb 31-41 %}

### 3. State flags and the wedge

`centered`, `spinning`, `hover` and `frame` hold the live state, and `wedge` is a three-point `ConvexShape` whose transform will be supplied per draw rather than by `Transformable`.

{% example ruby examples/graphics/shapes_transforms/shapes_transforms.rb 43-52 %}

### 4. Read input in the event loop

`poll_events!` handles `R` (pivot toggle via `box.origin = centered ? box.size : [0.0, 0.0]`), `Space` (spin), wheel scaling with `box.scale!(factor)`, and `'mouse-moved'` hit-testing through `box.global_bounds.contains?`.

{% example ruby examples/graphics/shapes_transforms/shapes_transforms.rb 54-74 %}

### 5. Hold-keys, spin, and draw the box and bounds

`Keyboard.key_pressed?` polls keys held right now, moving the box with `box.move` and spinning it with `box.rotate(1)`. `box.global_bounds` is then redrawn as an unfilled `RectangleShape` coloured by the hover state.

{% example ruby examples/graphics/shapes_transforms/shapes_transforms.rb 76-92 %}

### 6. Draw the origin

A small `CircleShape` is placed at `position + origin`, making the pivot visible as it switches between corner and centre.

{% example ruby examples/graphics/shapes_transforms/shapes_transforms.rb 94-100 %}

### 7. Compose a `Transform` by hand

The bottom row builds `Transform.identity.translate(...).rotate(...).scale(...)`, installs it in a `RenderState`, and draws the shared `wedge` with `window.draw(wedge, state)` — the explicit form of what `Transformable` does for you.

{% example ruby examples/graphics/shapes_transforms/shapes_transforms.rb 102-112 %}

### 8. Draw the HUD and present

A single `Text` reports origin, rotation, scale, pivot, hover and the measured bounds; `window.display!` swaps the frame and `frame` advances for the next transform.

{% example ruby examples/graphics/shapes_transforms/shapes_transforms.rb 114-124 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Window` | Window | OS window + render surface | `new`, `frame_rate=`, `draw`, `display!`, `clear!` |
| `Event` + `Keyboard` | Window | Input | `poll_events!`, `event.type`, `Keyboard.key_pressed?` |
| `Transformable` | Graphics | The mixin being demonstrated | `position`, `move`, `rotation`, `rotate`, `scale!`, `origin=`, `origin` |
| `RectangleShape` / `CircleShape` / `ConvexShape` | Graphics | The drawn geometry | `new`, `size`, `global_bounds`, `local_bounds`, `fill_color=`, `outline_*` |
| `Rect` | Graphics | Bounds value returned by the bound readers | `size`, `left`, `top`, `contains?` |
| `Transform` | Graphics | Composable 3×3 matrix | `identity`, `translate`, `rotate`, `scale` |
| `RenderState` | Graphics | Per-draw state (carries the transform) | `new`, `transform=` |

See the [Graphics API]({% link api/graphics.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/graphics/shapes_transforms/shapes_transforms.rb` and run.

{% example ruby examples/graphics/shapes_transforms/shapes_transforms.rb %}

{: .note }
> `origin` is the pivot for rotation and scale — default `(0, 0)` (top-left). Use `local_bounds` for layout and `global_bounds` for hit-testing; `a * b` on a `Transform` means `a` applied after `b`.
