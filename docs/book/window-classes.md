---
layout: default
title: Window
parent: "Part I — Essential"
grand_parent: Book
nav_order: 2
---

# Recipe: Window

The three window classes side by side. `WindowBase` is an OS window and event queue with no OpenGL context — it shows but cannot clear, draw or display. `Window` adds the context and stays directly renderable (`clear!`/`display!`/`draw`). `RenderWindow` adds the full `RenderTarget` surface. The demo opens all three, animates the two renderable ones, and retitles the `WindowBase` each frame with its live size.

> **Essential**

## Goal

Three windows in a row labelled WindowBase, Window and RenderWindow. The `Window` and `RenderWindow` each show a spinning circle; the `WindowBase` shows its live size and focus state in its title bar. Closing any one (or pressing escape) closes all three.

## How the code works

### 1. Load SFML and the window subsystems

`require 'sfml'` pulls in the whole binding, and the three `include` lines bring `Window`, `Graphics` and `System` into scope so the classes can be named without their `SF::…` prefixes.

{% example ruby examples/window/window_classes/window_classes.rb 14-17 %}

### 2. A shared font and text helper

`Font.from_file` loads the face once into `FONT`, and the `text` helper wraps the repeated `Text.new` → `fill_color=` → `position=` sequence so each window can be labelled with one call.

{% example ruby examples/window/window_classes/window_classes.rb 20-28 %}

### 3. Open all three without a block

`WindowBase.open`, `Window.open` and `RenderWindow.open` are the block-less constructors: they return the window already open, so all three coexist. The block form would close the window when its block returned.

{% example ruby examples/window/window_classes/window_classes.rb 35-40 %}

### 4. Place them side by side

Each class inherits `position=`, which moves the native OS window. Offsetting the three x-coordinates makes them appear in a row.

{% example ruby examples/window/window_classes/window_classes.rb 42-46 %}

### 5. Build the shared circle

One `CircleShape` with a centred `origin` is reused by both renderable windows; `rotation` starts at zero and is advanced in the loop.

{% example ruby examples/window/window_classes/window_classes.rb 48-54 %}

### 6. One loop, three event queues

The loop runs while no window has closed. Each `WindowBase` owns its own queue, so `windows.each { … poll_events! … }` polls all three; a `'closed'` event or `:escape` from any one calls `windows.each(&:close!)`.

{% example ruby examples/window/window_classes/window_classes.rb 56-63 %}

### 7. Render the two windows that have a context

`Window` and `RenderWindow` both support `clear!`, `draw` and `display!`; the circle's rotation is set independently for each so you can see they draw separately.

{% example ruby examples/window/window_classes/window_classes.rb 65-79 %}

### 8. `WindowBase` reports, then clean up

With no OpenGL context, `WindowBase` cannot draw, so it only writes its live size and focus state into `title=`. After the loop, `windows.each(&:close!)` releases every native window.

{% example ruby examples/window/window_classes/window_classes.rb 81-85 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `WindowBase` | Window | Events + native surface only | `open`, `position=`, `title=`, `size`, `focused?`, `open?`, `close!`, `poll_events!` |
| `Window` | Window | `WindowBase` + GL context, directly renderable | `clear!`, `draw`, `display!` |
| `RenderWindow` | Window | `Window` + full `RenderTarget` surface | `clear!`, `draw`, `display!` |
| `VideoMode` | Window | Size + bit depth | `new` |
| `CircleShape` | Graphics | The shared drawn shape | `new`, `origin=`, `rotation=` |
| `Text` / `Font` | Graphics | Per-window label | `Font.from_file`, `Text.new`, `position=` |

See the [Window API]({% link api/window.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/window/window_classes/window_classes.rb` and run.

{% example ruby examples/window/window_classes/window_classes.rb %}

{: .note }
> Closing any window (or escape) closes all three. The hierarchy is `RenderWindow < Window < WindowBase`; pick `WindowBase` when you only need events and a native surface, and `RenderWindow` when you need to draw.
