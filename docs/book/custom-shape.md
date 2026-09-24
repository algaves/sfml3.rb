---
layout: default
title: Custom Shape
parent: "Part III — Intermediate"
grand_parent: Book
nav_order: 7
---

# Recipe: Custom Shape

`Shape` is the base class behind `CircleShape`, `RectangleShape` and `ConvexShape`. Subclass it and answer two questions — `#point_count` and `#point(index)` — and the base class builds the polygon, styles it and gives you `Transformable` and `Drawable` for free. This is how you make a star, a gear, a heart, or any outline the built-ins do not cover.

> **Intermediate**

## Goal

An 800×600 window with a spinning ten-pointed star (a `Shape` subclass) and a larger hollow star rotating the other way. `+`/`-` change the radius, `[`/`]` change the number of points, space swaps fill for outline. Escape quits.

## How the code works

### 1. Subclass Shape

`Star < Shape` calls `super()` so it inherits the base styling plus `Transformable` and `Drawable`, and stores the radius and spike count. The `update!` at the end of `initialize` builds the polygon from the methods below.

{% example ruby examples/subsystems/custom_shape/custom_shape.rb 20-28 %}

### 2. Answer the two geometry questions

`point_count` is twice the spike count, one corner per spike plus one per valley. `point(index)` spaces the corners around a full turn and uses the full radius on even indices and `0.45 *` it on odd ones, which is what makes the star shape.

{% example ruby examples/subsystems/custom_shape/custom_shape.rb 30-38 %}

### 3. Refresh with regrow!

`regrow!` just calls `update!` again. Because the base class reads `point_count` and `point` only when refreshed, every later geometry change has to go through this.

{% example ruby examples/subsystems/custom_shape/custom_shape.rb 40-42 %}

### 4. Create the spinning pair

The window runs at 60 fps, and two `Star`s are placed at its centre: a smaller filled one and a larger hollow one with an outline. `filled` remembers which look the yellow star should have.

{% example ruby examples/subsystems/custom_shape/custom_shape.rb 45-59 %}

### 5. Toggle fill and outline

Space flips `filled` and rewrites the filled star's `fill_color` and `outline_thickness`, so the swap between solid gold and a gold outline is visible immediately.

{% example ruby examples/subsystems/custom_shape/custom_shape.rb 67-74 %}

### 6. Change radius and spikes

`+`/`-` add or remove ten units of radius (keeping a minimum) and `[`/`]` add or remove a spike between three and twelve. Each change is followed by `regrow!` so the polygon is rebuilt at the new size.

{% example ruby examples/subsystems/custom_shape/custom_shape.rb 75-86 %}

### 7. Spin, draw and present

The filled star gains one degree of rotation per frame and the outline half a degree in the opposite direction. After `clear!`, both are drawn and `display!` presents the frame.

{% example ruby examples/subsystems/custom_shape/custom_shape.rb 91-97 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Shape` | Graphics | Base class to subclass | `initialize`, `point_count`, `point`, `update!` |
| `Transformable` | Graphics | Inherited by `Shape` | `position=`, `origin=`, `rotation`, `rotation=` |
| `Drawable` | Graphics | Inherited by `Shape` | `draw` |
| `Window` | Window | Render surface | `new`, `frame_rate=`, `poll_events!`, `draw`, `display!` |
| `Event` | Window | Key handling | `type`, `code` |

See the [Graphics API]({% link api/graphics.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/subsystems/custom_shape/custom_shape.rb` and run.

{% example ruby examples/subsystems/custom_shape/custom_shape.rb %}

{: .note }
> Custom `Shape` subclasses must call `update!` after any geometry change. For geometry that is not a closed styled polygon — a fountain of sparks, a batched mesh — use a [`VertexArray`]({% link book/vertex-arrays.md %}) instead, as the [Particle System]({% link book/particles.md %}) does.
