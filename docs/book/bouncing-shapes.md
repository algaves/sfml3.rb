---
layout: default
title: Bouncing Shapes
parent: "Part II — Basic"
grand_parent: Book
nav_order: 1
---

# Recipe: Bouncing Shapes

The same five components as [Hello Shapes]({% link book/hello-shapes.md %}) — Window, Events, Transformable, Drawable objects, primitive shapes — reinterpreted as an interactive app: shapes bounce off the window edges, the mouse grabs and drags one, the wheel rescales them, space spawns a fresh shape and escape quits.

> **Basic**

## Goal

Four randomly-shaped, randomly-coloured shapes drifting around a 640×480 window. They bounce off all four edges, keep spinning, can be dragged with the left mouse button, scaled with the wheel, and added to with space. Escape or the close button quits.

## How the code works

### 1. Model the bouncer and build shapes

`Bouncer = Struct.new(:shape, :vx, :vy)` pairs each drawable with its per-axis velocity so the motion data travels with the shape it drives, and `MARGIN` is the radius used for edging and hit-testing alike. `new_shape` picks at random among `CircleShape`, `RectangleShape` and `ConvexShape`, then sets `position`, `origin` and a random `fill_color` from `random_color` before handing it back.

{% example ruby examples/bouncing_shapes/bouncing_shapes.rb 17-32 %}

### 2. Bounce off an edge

`bounce(axis, velocity, radius, limit)` returns a corrected `[coordinate, velocity]` pair: past the far edge it clamps to `limit - radius` and reverses the velocity, before the near edge it clamps to `radius` and sends it forward, otherwise it returns the inputs unchanged. Keeping it a pure function keeps the edge logic easy to read and test.

{% example ruby examples/bouncing_shapes/bouncing_shapes.rb 34-42 %}

### 3. Hit-test and spawn

`near?` accepts a bouncer when the cursor is within `MARGIN` on both axes, which is how the drag finds a target. `spawn` builds a fresh shape, drops it at the window centre and appends a `Bouncer` with a random velocity; this is what the space bar calls.

{% example ruby examples/bouncing_shapes/bouncing_shapes.rb 44-53 %}

### 4. Open the window and seed the bouncers

The `Window` is created and capped at 60 fps before the four starting bouncers are laid across the middle row with independent random velocities. `held` and `grab_offset` are the drag state: which bouncer the cursor owns and the vector from the cursor to its centre.

{% example ruby examples/bouncing_shapes/bouncing_shapes.rb 55-65 %}

### 5. Handle key events

`poll_events!` runs the queue each frame. `'closed'` and the escape key both call `close!`, and the space key calls `spawn` to add another bouncer at the centre.

{% example ruby examples/bouncing_shapes/bouncing_shapes.rb 67-77 %}

### 6. Handle mouse events

A left press records the bouncer under the cursor and the offset from its centre; release drops it; move writes the cursor position plus that offset back to the shape while held; and the wheel scales every shape by 1.1 or 0.9.

{% example ruby examples/bouncing_shapes/bouncing_shapes.rb 78-97 %}

### 7. Advance the un-held shapes

Each bouncer that is not the one being dragged is stepped: `bounce` corrects both axes against the window size, the corrected position is written back and the shape spins. The dragged shape is skipped so the cursor owns it until release.

{% example ruby examples/bouncing_shapes/bouncing_shapes.rb 100-111 %}

### 8. Draw the frame

`clear!` erases the previous frame, every `bouncer.shape` is drawn, and `display!` swaps the finished frame onto the screen before the loop re-checks `window.open?`.

{% example ruby examples/bouncing_shapes/bouncing_shapes.rb 113-115 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Window` | Window | OS window + render surface | `new`, `frame_rate=`, `size`, `open?`, `close!`, `draw`, `display!`, `clear!` |
| `Event` | Window | Queued input events | `type`, `code`, `mouse_button`, `mouse_move`, `mouse_wheel_scroll` |
| `Struct` (Ruby) | — | Holds shape + velocity | `new`, `vx`, `vy` |
| `Transformable` | Graphics | position/rotation/scale/origin | `position`, `position=`, `scale!`, `rotate`, `origin=` |
| `CircleShape` / `RectangleShape` / `ConvexShape` | Graphics | The randomised primitives | `new`, `fill_color=` |

See the [Window API]({% link api/window.md %}) and [Graphics API]({% link api/graphics.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/bouncing_shapes/bouncing_shapes.rb` and run.

{% example ruby examples/bouncing_shapes/bouncing_shapes.rb %}

{: .note }
> Run it under `xvfb-run -a` headlessly. The bounce is plain arithmetic on `shape.position`; hit-testing the drag uses a fixed margin around each shape's position.
