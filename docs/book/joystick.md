---
layout: default
title: Joystick
parent: "Part III — Intermediate"
grand_parent: Book
nav_order: 1
---

# Recipe: Joystick

`SF::Window::Joystick`: the real-time state of every gamepad. `Joystick.update!` refreshes the cached state, `connected?`/`button_count`/`axis?` describe what is plugged in, `axis_position` and `button_pressed?` read it, and `identification` returns the name and vendor/product ids. `R` rescans for hot-plugging.

> **Intermediate**

## Goal

A 680×440 window that draws the first connected gamepad: its name and vendor/product ids, a bar per supported axis with a moving knob and a numeric read-out, and a grid of button indicators that light when pressed. With nothing connected it shows instructions. `R` rescans; escape quits.

## How the code works

### 1. Setup helpers and constants

`Font.from_file` loads the one font used for every label, and `text` is a small helper that builds a styled `Text` at a position so each draw call stays a single expression. `AXES` lists the eight axis names the library exposes.

{% example ruby examples/subsystems/joystick/joystick.rb 21-29 %}

### 2. Draw an axis row

For each axis the device actually has (`Joystick.axis?`), a label, a dark track, a centre tick, a knob and a numeric read-out are drawn. `Joystick.axis_position` returns `-100.0`–`100.0`, which is scaled onto the track as the knob's x position; the knob brightens once the value leaves the dead zone.

{% example ruby examples/subsystems/joystick/joystick.rb 31-53 %}

### 3. Draw the button grid

`Joystick.button_count` says how many buttons exist, and the loop lays them out eight per row. `Joystick.button_pressed?` chooses a green fill for a held button and a dark fill otherwise.

{% example ruby examples/subsystems/joystick/joystick.rb 55-68 %}

### 4. Describe the connected pad

`Joystick.identification(id)` returns the name plus vendor and product ids, printed as a two-line header. `draw_axes` and `draw_buttons` then render the rest of the panel.

{% example ruby examples/subsystems/joystick/joystick.rb 70-78 %}

### 5. Open the window and poll events

The window is capped at 60 fps and `connected` starts as `nil`. Each frame `poll_events!` handles the close button and Escape, clears `connected` on `R` to force a rescan, and does the same when the queue reports a joystick being plugged in or removed.

{% example ruby examples/subsystems/joystick/joystick.rb 80-95 %}

### 6. Refresh the cache and pick a device

The state is a snapshot, so `Joystick.update!` must run once per frame before any read. The `||=` then scans the fixed slots with `Joystick.connected?` and caches the first id it finds.

{% example ruby examples/subsystems/joystick/joystick.rb 97-98 %}

### 7. Draw the device or a fallback

With an id cached, `draw_joystick` paints the full panel; otherwise an explanation of the slot and button limits is shown instead. `display!` swaps the finished frame onto the screen.

{% example ruby examples/subsystems/joystick/joystick.rb 100-111 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Joystick` | Window | Gamepad state | `update!`, `connected?`, `identification`, `axis?`, `axis_position`, `button_count`, `button_pressed?`, `COUNT`, `BUTTON_COUNT`, `AXIS_COUNT` |
| `Event` | Window | Connect/disconnect notifications | `type`, `code` |
| `RectangleShape` | Graphics | Axis tracks and button cells | `new`, `position=`, `fill_color=`, `outline_*` |
| `CircleShape` | Graphics | Axis knobs | `new`, `origin=`, `position=`, `fill_color=` |
| `Text` / `Font` | Graphics | Names, axes and values | `Font.from_file`, `Text.new` |

See the [Input API]({% link api/input.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/subsystems/joystick/joystick.rb` and run.

{% example ruby examples/subsystems/joystick/joystick.rb %}

{: .note }
> Always guard with `connected?`. `axis_position` is noisy around zero — apply a deadzone with `WindowBase#joystick_threshold=`. The event queue also reports connect/disconnect.
