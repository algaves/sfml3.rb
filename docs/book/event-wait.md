---
layout: default
title: Events
parent: "Part I — Essential"
grand_parent: Book
nav_order: 3
---

# Recipe: Events

SFML reports everything the window and input devices do as **events** on a queue: a key press, a mouse move, a resize, a text character, a touch, a gamepad change. Each event has a type string and — for the kinds that carry data — a payload hash, and every kind also has a predicate method so you can branch without matching strings.

> **Essential**

## Goal

A 900×560 window that logs the last fourteen events, one line each, with their payloads, over a marker that keeps spinning to prove the loop is not blocked. Move, click, scroll, type, resize, plug in a gamepad or touch the screen and watch the log. Escape quits.

## How the code works

### 1. Load SFML and the window subsystems

`require 'sfml'` plus the three `include` lines bring the event, graphics and clock classes into scope without their `SF::…` prefixes.

{% example ruby examples/subsystems/event_types/event_types.rb 16-19 %}

### 2. A shared font and text helper

`Font.from_file` loads the face once, and `text` packages the `Text.new` → `fill_color=` → `position=` sequence so each log line is one call.

{% example ruby examples/subsystems/event_types/event_types.rb 22-30 %}

### 3. Turn one event into a readable line

`describe` branches on the `*?` predicates rather than the type string: `closed?`, `resized?`, focus and text events, then the key, wheel and mouse-button cases, reading each payload through `event.code`, `event.mouse_wheel_scroll` and `event.mouse_button`.

{% example ruby examples/subsystems/event_types/event_types.rb 34-46 %}

### 4. Cover the input devices too

The same dispatch continues through mouse-move (raw and window), mouse-enter/leave, joystick connect/move/button, touch and sensor events.

{% example ruby examples/subsystems/event_types/event_types.rb 48-63 %}

### 5. Fall back and read payload positions

An unrecognised event returns `"#{event.type} (no payload)"`, and `button_position`/`touch_position` unpack the `:x`/`:y` payload values for the log line.

{% example ruby examples/subsystems/event_types/event_types.rb 65-78 %}

### 6. Open the window and the spinner/log state

The window is opened and frame-capped, a `Clock` drives the spinner, and `log` starts with the waiting message.

{% example ruby examples/subsystems/event_types/event_types.rb 80-87 %}

### 7. Poll the queue and keep the last fourteen

`poll_events!` yields every queued event at once; `'closed'` or `:escape` closes the window, otherwise `describe(event)` is pushed to the front and the log is trimmed to fourteen lines.

{% example ruby examples/subsystems/event_types/event_types.rb 89-96 %}

### 8. Draw the spinner and the log

A `VertexArray` of `:lines` becomes a continuously rotating marker from `spinner.elapsed_time`, and a `Text` renders the log newest-first, with the escape hint near the bottom.

{% example ruby examples/subsystems/event_types/event_types.rb 98-115 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Event` | Window | One queued notification | `type`, `code`, the `*?` predicates, `key` / `mouse_button` / `mouse_move` / `mouse_wheel_scroll` / `touch` / `joystick_*` / `sensor` |
| `Window` | Window | Owns the event queue | `poll_events!`, `poll_event!`, `wait_event!`, `close!` |
| `Clock` | System | Drives the spinner | `new`, `elapsed_time` |
| `VertexArray` / `Vertex` | Graphics | The spinner line | `new`, `primitive=`, `append` |
| `Text` / `Font` | Graphics | The event log | `Text.new`, `position=` |

See the [Window API]({% link api/window.md %}) and the [Handling Input]({% link learn/handling-input.md %}) tutorial for more detail.

## The complete script

The complete program, ready to copy into `examples/subsystems/event_types/event_types.rb` and run.

{% example ruby examples/subsystems/event_types/event_types.rb %}

{: .note }
> This script polls, so animation runs between events. The companion [`event_wait` example](https://github.com/algaves/sfml3.rb/tree/main/examples/subsystems/event_wait) runs the same scene in both modes and times the block; `poll_events!` also returns an Enumerator without a block, and there is no plural `wait_events!`.
