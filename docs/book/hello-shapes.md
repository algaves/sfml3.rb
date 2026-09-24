---
layout: default
title: Hello Shapes
parent: "Part I — Essential"
grand_parent: Book
nav_order: 1
---

# Recipe: Hello Shapes

The five building blocks every example is built from — a `Window`, the events it polls, the `Transformable` mixin, `Drawable` objects and the primitive shapes — used once, with nothing else (no clock, views or textures), so each component reads in isolation.

> **Essential**

## Goal

A 640×480 window showing a red circle, a blue square and a white-outlined triangle. The circle and square spin in opposite directions and the triangle tracks the circle's `position`, so the update step is visible. Close the window or press Escape to quit.

## How the code works

### 1. Open the window

`Window.new(VideoMode.new(640, 480, 32), 'SFML hello shapes')` creates the OS window and its OpenGL surface together; `window.frame_rate = 60` caps it so the CPU is not burned at hundreds of frames a second.

{% example ruby examples/hello_shapes/hello_shapes.rb 17-18 %}

### 2. Create, place and style the shapes

`CircleShape.new(60)` and `RectangleShape.new([140, 140])` take their size directly; `ConvexShape.new(3)` starts empty and each corner is added with `set_point(index, [x, y])`. `position=` is the draw position and `origin=` the pivot the shape rotates about (so the circle spins about its centre). `fill_color=` fills the interior, `outline_thickness=`/`outline_color=` draw a border, and the three are collected into one array to draw together.

{% example ruby examples/hello_shapes/hello_shapes.rb 20-39 %}

### 3. Poll events first

Every frame begins with `window.poll_events! { |event| … }`. The block sees each queued event in order: `'closed'` is the close button, and `:escape` on a key press is Escape. Both call `window.close!`.

{% example ruby examples/hello_shapes/hello_shapes.rb 41-47 %}

### 4. Update

`circle.rotate(1)` and `square.rotate(-1)` add rotation in degrees; the triangle is repositioned from the circle's `position`, which is why it appears to follow it. Doing updates before drawing keeps the frame coherent.

{% example ruby examples/hello_shapes/hello_shapes.rb 49-51 %}

### 5. Draw and present

`window.clear!` erases the previous frame, `window.draw(shape)` queues a shape, and `window.display!` swaps the finished frame onto the screen. The loop then re-checks `window.open?`, which turns false after `close!`.

{% example ruby examples/hello_shapes/hello_shapes.rb 53-56 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Window` | Window | OS window + render surface | `new`, `frame_rate=`, `open?`, `close!`, `clear!`, `draw`, `display!` |
| `VideoMode` | Window | Size and bit depth for a window | `new` |
| `Event` | Window | One queued input/window notification | `type`, `code`, `closed?`, `key_pressed?` |
| `Transformable` | Graphics | Shared position/rotation/scale/origin mixin | `position=`, `origin=`, `rotate`, `move` |
| `Drawable` | Graphics | Anything `Window#draw` accepts | `draw(target, state)` |
| `CircleShape` / `RectangleShape` / `ConvexShape` | Graphics | The three primitive shapes | `new`, `set_point`, `fill_color=`, `outline_thickness=`, `outline_color=` |

See the [Window API]({% link api/window.md %}) and [Graphics API]({% link api/graphics.md %}) for the full signature of each.

## The complete script

{% example ruby examples/hello_shapes/hello_shapes.rb %}

{: .note }
> Run it with `bundle exec ruby -Ilib examples/hello_shapes/hello_shapes.rb`. The whole loop is `Window#frame_rate=`, `poll_events!` and `event.code == :escape`. Continue to [Bouncing Shapes]({% link book/bouncing-shapes.md %}) to see the same parts made interactive.
