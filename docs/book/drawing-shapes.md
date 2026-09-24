---
layout: default
title: Shape
parent: "Part I — Essential"
grand_parent: Book
nav_order: 5
---

# Recipe: Shape

The three built-in shapes — `CircleShape`, `RectangleShape` and `ConvexShape` — with the styling they share (`fill_color`, `outline_thickness`, `outline_color`), the circle's `point_count` tessellation, the origin pivot, and texturing a shape instead of filling it.

> **Essential**

## Goal

A 900×560 window with a red spinning circle, a blue rectangle that can switch between fill and a generated checkerboard texture, and a green hexagon. Keys select a shape (its transformed bounds are outlined), change the circle's smoothness, toggle outlines, or texture the rectangle. Escape quits.

## How the code works

### 1. Load SFML and the shared text helper

`require 'sfml'` and the three `include` lines expose the classes, while `FONT` and `text` build every HUD label with one call.

{% example ruby examples/graphics/shapes_basic/shapes_basic.rb 14-28 %}

### 2. Make a CPU image and a tiled checker texture

`image` packs raw RGBA bytes into a binary string for `Image.from_pixels`, and `checker_texture` uploads it with `Texture.from_image` and sets `repeated = true` so it tiles.

{% example ruby examples/graphics/shapes_basic/shapes_basic.rb 30-50 %}

### 3. Open the window and create the circle and rectangle

The window must exist before any `Texture` is created. `CircleShape.new(70)` sets the radius and `point_count = 64` the tessellation; both shapes get a centred `origin`, a fill and an outline.

{% example ruby examples/graphics/shapes_basic/shapes_basic.rb 52-71 %}

### 4. Build the hexagon

`ConvexShape.new(6)` allocates six vertices, and a loop walks a full turn placing each with `set_point(index, [cos · 80, sin · 80])`. The result stays convex, which `ConvexShape` requires.

{% example ruby examples/graphics/shapes_basic/shapes_basic.rb 73-84 %}

### 5. Group the shapes and attach the texture

`shapes = [circle, rectangle, hexagon]` and `NAMES` let one loop style and draw all three. Assigning `rectangle.texture = checker` switches it from fill to texture, and `texture_rect` chooses the region to stretch over the shape.

{% example ruby examples/graphics/shapes_basic/shapes_basic.rb 86-95 %}

### 6. React to selection and toggle keys

`case event.code` maps `:num1`/`:num2`/`:num3` to `selected`, `:t` toggles every `outline_thickness` between 4 and 0, `:Space` toggles the rectangle texture, and `:Equal`/`:Hyphen` step `circle.point_count` between 8 and 64.

{% example ruby examples/graphics/shapes_basic/shapes_basic.rb 97-121 %}

### 7. Rotate, clear, and draw the shapes and bounds

Every shape is `rotate`d, `window.clear!` erases the frame, the shapes are drawn, and the selected shape's `global_bounds` is outlined with a transparent `RectangleShape`.

{% example ruby examples/graphics/shapes_basic/shapes_basic.rb 123-136 %}

### 8. Draw the labels and present

The labels row brackets the selected name, a status line reports tessellation, outlines and texture state, and `window.display!` presents the finished frame.

{% example ruby examples/graphics/shapes_basic/shapes_basic.rb 138-146 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Window` | Window | OS window + render surface | `new`, `frame_rate=`, `open?`, `clear!`, `draw`, `display!` |
| `VideoMode` | Window | Size + bit depth | `new` |
| `CircleShape` / `RectangleShape` / `ConvexShape` | Graphics | The three primitive shapes | `new`, `set_point`, `point_count=`, `position=`, `origin=`, `rotate`, `global_bounds` |
| `Shape` | Graphics | Base class the three inherit from | `fill_color=`, `outline_thickness=`, `outline_color=` |
| `Image` | Graphics | CPU pixel buffer | `from_pixels` |
| `Texture` | Graphics | GPU image, usable by a shape | `from_image`, `repeated=` |
| `Text` / `Font` | Graphics | HUD labels | `Font.from_file`, `Text.new`, `fill_color=`, `position=` |

See the [Graphics API]({% link api/graphics.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/graphics/shapes_basic/shapes_basic.rb` and run.

{% example ruby examples/graphics/shapes_basic/shapes_basic.rb %}

{: .note }
> Press 1/2/3 to select a shape (its `global_bounds` is outlined), `+`/`-` to change the circle tessellation, `T` for outlines, space to texture the rectangle. For pivots, bounds and transforms in depth see [Transformable]({% link book/shape-transforms.md %}).
