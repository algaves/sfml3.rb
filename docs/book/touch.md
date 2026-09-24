---
layout: default
title: Touch
parent: "Part III — Intermediate"
grand_parent: Book
nav_order: 2
---

# Recipe: Touch

`SF::Window::Touch`: mobile multi-touch by finger index. `down?` reports whether a finger is touching and `position(finger, relative_to: window)` reads it, alongside the `touch_began?`/`touch_moved?`/`touch_ended?` events.

> **Intermediate**

## Goal

A 640×420 window that draws each active finger as a coloured halo and dot labelled with its index, plus the last raw touch event. On a desktop with no touchscreen it stays empty — the code is written for a touch device.

## How the code works

### 1. Setup helpers

`Font.from_file` loads the font and `text` returns a styled `Text` in one call, so every label in the loop stays a single expression.

{% example ruby examples/subsystems/touch/touch.rb 22-28 %}

### 2. Ten tracked fingers

`FINGERS` is the fixed range of finger indices the library tracks, and `COLORS` gives each one a distinct colour. Both are constants because they never change while the program runs.

{% example ruby examples/subsystems/touch/touch.rb 30-32 %}

### 3. Open the window and read touch events

The window is opened at 60 fps and `last` holds the HUD string. `poll_events!` handles close and Escape, and on `'touch-began'`, `'touch-moved'` or `'touch-ended'` reads `event.touch` — a hash of `:finger` and `:position` — into `last`.

{% example ruby examples/subsystems/touch/touch.rb 34-50 %}

### 4. Draw the HUD

The header explains that the circles below come from the real-time query, and shows the most recent event so the event stream is visible even when it cannot be drawn.

{% example ruby examples/subsystems/touch/touch.rb 52-57 %}

### 5. Draw a halo and dot per finger

For each index, `Touch.down?` decides whether the finger is touching and `Touch.position(finger, window)` gives its window-relative point. A large translucent halo and a smaller opaque dot are centred on it.

{% example ruby examples/subsystems/touch/touch.rb 59-74 %}

### 6. Label each finger and present

The index is drawn on top of the dot and the full name beside it. `display!` swaps the finished frame onto the screen; on a desktop with no touchscreen the loop simply draws the HUD.

{% example ruby examples/subsystems/touch/touch.rb 76-83 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Touch` | Window | Per-finger state | `down?`, `position` |
| `Event` | Window | Touch phase + payload | `type`, `touch` |
| `CircleShape` | Graphics | Halo and dot per finger | `new`, `origin=`, `position=`, `fill_color=` |
| `Text` / `Font` | Graphics | Finger labels and HUD | `Font.from_file`, `Text.new`, `position=`, `color:` |
| `Window` | Window | Surface and event queue | `new`, `poll_events!`, `clear!`, `draw`, `display!` |

See the [Input API]({% link api/input.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/subsystems/touch/touch.rb` and run.

{% example ruby examples/subsystems/touch/touch.rb %}

{: .note }
> On desktop `down?` is almost always false. Finger indices are not stable across a touch end/start — re-resolve each gesture rather than caching an index.
