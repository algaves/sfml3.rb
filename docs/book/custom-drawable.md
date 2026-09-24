---
layout: default
title: Drawable
parent: "Part I — Essential"
grand_parent: Book
nav_order: 6
---

# Recipe: Drawable

The `Drawable` contract. Anything that mixes in `SF::Graphics::Drawable` and defines `#draw(target, state)` can be handed to `window.draw`, exactly like a built-in shape. Here a plain Ruby `Crosshair` composes a `CircleShape` and a `VertexArray` and forwards them to the render target it is given, so one `window.draw` draws both.

> **Essential**

## Goal

An 800×600 window where a green crosshair (a ring plus four tick marks) follows the mouse, next to a built-in yellow `CircleShape`. Moving the mouse steers the crosshair; clicking cycles its colour; escape quits.

## How the code works

### 1. Load SFML and the shared text helper

The `include` lines expose `Drawable`, the shapes and the window, and `FONT` plus the `text` helper build the HUD labels.

{% example ruby examples/graphics/custom_drawable/custom_drawable.rb 14-29 %}

### 2. Define `Crosshair` and its children

`class Crosshair; include SF::Graphics::Drawable` opts into the contract, and `initialize` builds the ring (`CircleShape`, transparent fill, green outline, centred origin) and the `VertexArray` of `:lines` that will hold the ticks.

{% example ruby examples/graphics/custom_drawable/custom_drawable.rb 31-51 %}

### 3. Move by rebuilding the ticks

`center=` stores the point, moves the ring, clears the tick array and appends two vertices per compass direction, using the ring's `outline_color` so the ticks match.

{% example ruby examples/graphics/custom_drawable/custom_drawable.rb 53-69 %}

### 4. Recolour and implement `#draw`

`recolor` changes the ring's outline and re-runs `center=` so the ticks pick up the colour; `#draw(target, state)` forwards both children to the render target it was given.

{% example ruby examples/graphics/custom_drawable/custom_drawable.rb 71-80 %}

### 5. Open the window and create the drawables

A `Crosshair` and a built-in `CircleShape` dot are created side by side, so the demo can prove `window.draw` treats them identically.

{% example ruby examples/graphics/custom_drawable/custom_drawable.rb 82-94 %}

### 6. A palette to cycle through

`PALETTE` holds four colours and `color_index` tracks the current one, wrapping with `%`.

{% example ruby examples/graphics/custom_drawable/custom_drawable.rb 96-100 %}

### 7. Drive it from events

`'mouse-moved'` sets `crosshair.center`, `'mouse-button-pressed'` advances the palette and calls `recolor`, and `'closed'`/`:escape` quit.

{% example ruby examples/graphics/custom_drawable/custom_drawable.rb 102-115 %}

### 8. Draw both drawables and the HUD

`window.draw(crosshair)` emits the whole custom object (ring plus ticks) and `window.draw(dot)` emits the built-in shape through the same interface; the HUD states the point.

{% example ruby examples/graphics/custom_drawable/custom_drawable.rb 117-130 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Drawable` | Graphics | The mixin being implemented | `draw(target, state)` |
| `Target` | Graphics | Render target passed to `#draw` | `draw` |
| `CircleShape` | Graphics | Child drawable (the ring) | `new`, `origin=`, `position=`, `outline_*`, `fill_color=` |
| `VertexArray` / `Vertex` | Graphics | Child drawable (the tick marks) | `new`, `primitive=`, `clear!`, `append` |
| `Window` | Window | Window + render target | `new`, `poll_events!`, `draw`, `display!`, `clear!` |
| `Event` | Window | Mouse input | `mouse_move`, `mouse_button_pressed?` |

See the [Graphics API]({% link api/graphics.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/graphics/custom_drawable/custom_drawable.rb` and run.

{% example ruby examples/graphics/custom_drawable/custom_drawable.rb %}

{: .note }
> `#draw` receives the target, so a custom drawable can itself draw other drawables. For geometry defined by `#point_count`/`#point` instead, see [Custom Shape]({% link book/custom-shape.md %}).
