---
layout: default
title: Mouse
parent: "Part II — Basic"
grand_parent: Book
nav_order: 3
---

# Recipe: Mouse

`SF::Window::Mouse`: the real-time state of the pointer, independent of the event queue. `Mouse.position(window)` is relative to the window; `Mouse.position` with no argument is relative to the desktop. Click to leave a ripple and `C` recentres the pointer with `Mouse.set_position`.

> **Basic**

## Goal

A 720×480 window with a crosshair that tracks the pointer, a ripple that expands wherever you click, and a live panel listing which of the five buttons are held. `C` recentres the pointer, escape quits.

## How the code works

### 1. Load the font and build labels

`ASSETS` locates the example folder and `FONT` loads the bundled font once. The `text` helper wraps `Text.new` with a colour and position so HUD labels can be created where they are drawn.

{% example ruby examples/subsystems/mouse/mouse.rb 19-27 %}

### 2. Lines, the ripple model and the button set

`line` packs point pairs into a `VertexArray` and sets `primitive = :lines`, which is how arbitrary line geometry is drawn. `Ripple` stores a spawn point, radius and remaining life, and `button_colors` gives each of the five `BUTTONS` a distinct green value for the panel.

{% example ruby examples/subsystems/mouse/mouse.rb 29-42 %}

### 3. The HUD and crosshair

`draw_hud` prints the pointer in both coordinate spaces — `Mouse.position(window)` relative to the window and `Mouse.position` relative to the desktop — plus the held buttons. It then draws two `line` calls through the window-relative point to form the crosshair.

{% example ruby examples/subsystems/mouse/mouse.rb 44-58 %}

### 4. Open the window and track state

The window is created and capped at 60 fps, and `ripples` holds the active rings while `colors` caches the per-button panel colours.

{% example ruby examples/subsystems/mouse/mouse.rb 60-63 %}

### 5. Handle events

`'closed'` and `:escape` close the window. `:c` recentres the pointer with `Mouse.set_position`, passing the window so the middle of its client area is the target, and a `'mouse-button-pressed'` event spawns a `Ripple` from the click position carried in `event.mouse_button[:position]`.

{% example ruby examples/subsystems/mouse/mouse.rb 65-78 %}

### 6. Advance the ripples

Every ripple grows by 2.5 px and fades by 6 alpha each frame, and `reject!` removes the ones whose life has run out so the list does not grow forever.

{% example ruby examples/subsystems/mouse/mouse.rb 80-84 %}

### 7. Draw the ripples

Each ripple becomes an unfilled `CircleShape`: transparent fill, a 2-pixel outline whose alpha is the current `life`, positioned on its spawn point. This is what makes the ring appear to pulse outward and fade.

{% example ruby examples/subsystems/mouse/mouse.rb 86-96 %}

### 8. The button panel and present

Every button in `BUTTONS` gets a box, filled with its colour when `Mouse.button_pressed?` reports it held — real-time state, not an event — and labelled. The HUD and crosshair are drawn last and `display!` presents the frame.

{% example ruby examples/subsystems/mouse/mouse.rb 98-110 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Mouse` | Window | Pointer position and button state | `position`, `set_position`, `button_pressed?`, `pressed?` |
| `Event` | Window | Click position | `type`, `mouse_button`, `code` |
| `CircleShape` | Graphics | Ripple rings | `new`, `origin=`, `position=`, `fill_color=`, `outline_*` |
| `RectangleShape` | Graphics | Button-state panel | `new`, `position=`, `fill_color=` |
| `VertexArray` / `Vertex` | Graphics | Crosshair lines | `new`, `primitive=`, `append` |
| `Struct` (Ruby) | — | Ripple state | `new`, `radius`, `life` |

See the [Input API]({% link api/input.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/subsystems/mouse/mouse.rb` and run.

{% example ruby examples/subsystems/mouse/mouse.rb %}

{: .note }
> Combine `Mouse.position(window)` with `window.map_pixel_to_coords` to pick under a custom `View`. Buttons are `:left`, `:right`, `:middle`, `:extra1`, `:extra2`, and `Mouse.button_pressed?` answers "held now?" while `mouse_button_pressed?` events answer "just happened".
