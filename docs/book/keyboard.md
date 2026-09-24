---
layout: default
title: Keyboard
parent: "Part II — Basic"
grand_parent: Book
nav_order: 2
---

# Recipe: Keyboard

`SF::Window::Keyboard`: real-time key state and the mapping between physical scancodes and logical key codes. `key_pressed?` takes a key Symbol, a name String or an Integer code; `scancode_pressed?` takes a physical scancode. The HUD shows the last key event and the `localize`/`delocalize`/`description` round-trips.

> **Basic**

## Goal

A 520×380 window with an on-screen keyboard where each key lights up while it is held, a readout of the last key event (code, scancode, modifiers, description), and a live list of every key currently down. `T` toggles the OS virtual keyboard (a no-op on most desktops); escape quits.

## How the code works

### 1. Load the font and build labels

`ASSETS` points at the example's own folder and `FONT` loads the bundled font once. The `text` helper wraps `Text.new`, sets its colour and position, and returns it, so labels can be created inline wherever they are drawn.

{% example ruby examples/subsystems/keyboard/keyboard.rb 19-28 %}

### 2. The QWERTY layout and key metrics

`ROWS` holds the keyboard as Symbols, one array per row, frozen so it doubles as the source of truth for both layout and state. `KEY_SIZE` and `KEY_GAP` fix the cell dimensions every later position is computed from.

{% example ruby examples/subsystems/keyboard/keyboard.rb 30-39 %}

### 3. A key-box helper

`key_box` builds one `RectangleShape` at a position with a single-pixel outline. It leaves the fill colour unset so `draw_keyboard` can decide it from the key's live state.

{% example ruby examples/subsystems/keyboard/keyboard.rb 41-47 %}

### 4. Draw the keyboard

For each row the `offset` centres it against the widest row, then a box is placed per key and filled blue when `Keyboard.key_pressed?(key)` reports it held, otherwise dark. The key's name is drawn on top as a small label.

{% example ruby examples/subsystems/keyboard/keyboard.rb 49-60 %}

### 5. Open the window and track state

The window is created and capped at 60 fps; `last` starts the HUD readout and `virtual_keyboard` mirrors the OS on-screen keyboard flag.

{% example ruby examples/subsystems/keyboard/keyboard.rb 62-65 %}

### 6. Handle key events

`poll_events!` gives `event.key`, a hash carrying `:code` (the logical key), `:scancode` (the physical one) and the `:shift`/`:control`/`:alt` modifiers. `'closed'` and `:escape` close, `:t` toggles `Keyboard.virtual_keyboard_visible`, and `localize`, `delocalize` and `description` are combined into the readout so the scancode/key round-trip is visible.

{% example ruby examples/subsystems/keyboard/keyboard.rb 67-85 %}

### 7. Held keys and the HUD

`ROWS.flatten.select` asks `Keyboard.key_pressed?` for every key to build the live "pressed now" list without touching events. The frame then clears, draws the keyboard and the HUD, and presents.

{% example ruby examples/subsystems/keyboard/keyboard.rb 87-96 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Keyboard` | Window | Key state and scancode/keycode mapping | `key_pressed?`, `scancode_pressed?`, `localize`, `delocalize`, `description`, `virtual_keyboard_visible=` |
| `Event` | Window | Per-key event with modifiers | `type`, `key` |
| `RectangleShape` | Graphics | One box per on-screen key | `new`, `position=`, `fill_color=`, `outline_*` |
| `Text` / `Font` | Graphics | Key labels and HUD | `Font.from_file`, `Text.new` |
| `Window` | Window | Surface and event queue | `new`, `poll_events!`, `clear!`, `draw`, `display!` |

See the [Input API]({% link api/input.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/subsystems/keyboard/keyboard.rb` and run.

{% example ruby examples/subsystems/keyboard/keyboard.rb %}

{: .note }
> Real-time `key_pressed?` ignores key-repeat — for text entry use `text_entered?` events. Special keys are capitalised in this binding: `:escape`, `:Space`, `:Left`, `:Enter`; letters and digits are lower-case (`:a`, `:num1`).
