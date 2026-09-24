---
layout: default
title: Menus & HUD
parent: GUI
grand_parent: "Part IV — Advanced"
nav_order: 1
---

# Recipe: Menus & HUD

An immediate-mode menu: buttons are data, their rectangles are recomputed each frame, and a click or keypress runs an action. The same pattern makes an options screen and an in-game HUD, so this recipe covers all three screens in one loop.

> **Advanced**

## Goal

A 900×620 window with three screens. The menu has three buttons (New Game, Options, Quit) that respond to both mouse and keyboard; Options has a volume slider; Playing shows a HUD with an elapsed timer. Up/Down or the mouse select, Enter or a click activates, escape backs out of a screen and quits from the menu.

## How the code works

### 1. Describe buttons as data and give them bounds

A `Button` `Struct` holds only a label and an action symbol, and `button_bounds(index)` computes a `Rect` from the index. The whole menu is therefore a list plus a formula, with no widget class in sight.

{% example ruby examples/subsystems/gui_menus/gui_menus.rb 34-47 %}

### 2. Seed the screen state

`state` picks which screen draws, `selected` is the keyboard cursor, `volume` backs the options slider, and `play_clock` measures play time. All four are plain locals the loop mutates.

{% example ruby examples/subsystems/gui_menus/gui_menus.rb 49-52 %}

### 3. Recompute the hover every frame

`Mouse.position(window)` is measured against each button's rectangle with `contains?` to find the hovered index. Because this happens from scratch each frame, the highlight always reflects the current pointer — the essence of immediate mode.

{% example ruby examples/subsystems/gui_menus/gui_menus.rb 54-56 %}

### 4. One action lambda changes state

The `activate` lambda maps an action symbol to a state change: `:new_game` switches to playing and restarts the clock, `:options` opens settings and `:quit` closes the window. Both input paths call it, so mouse and keyboard share one behaviour.

{% example ruby examples/subsystems/gui_menus/gui_menus.rb 58-68 %}

### 5. Poll clicks and keys

`poll_events!` activates the hovered button on a left `'mouse-button-pressed'`, walks `selected` with Up/Down, activates on Enter, and moves the volume with Left/Right only on the options screen; escape backs out of a screen or quits from the menu.

{% example ruby examples/subsystems/gui_menus/gui_menus.rb 70-87 %}

### 6. Draw the menu screen

The menu draws its title, then loops over the buttons: each rectangle becomes a `RectangleShape`, filled highlighted when it is hovered or selected, with its `Text` label drawn on top.

{% example ruby examples/subsystems/gui_menus/gui_menus.rb 91-104 %}

### 7. Draw the options slider

The options screen draws a track `RectangleShape` and a knob positioned at `left + volume * 3`, then a hint line. The widget is just arithmetic over the stored value, rebuilt every frame.

{% example ruby examples/subsystems/gui_menus/gui_menus.rb 105-116 %}

### 8. Draw the playing HUD

The playing screen draws a title and an elapsed time read from `play_clock.elapsed_time`, independent of the menu widgets. Escape returns to the menu, and `window.display!` presents the finished frame.

{% example ruby examples/subsystems/gui_menus/gui_menus.rb 117-121 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Mouse` / `Event` | Window | Pointer hover and clicks | `Mouse.position`, `event.mouse_button` |
| `RectangleShape` | Graphics | Buttons, slider track and knob | `new`, `position=`, `fill_color=`, `outline_thickness=` |
| `Text` / `Font` | Graphics | Labels and HUD | `Text.new`, `position=` |
| `Rect` | Graphics | Hit-testing a button | `new`, `contains?`, `left`, `top`, `width`, `height` |
| `Clock` | System | Elapsed play time | `new`, `restart!`, `elapsed_time` |
| `Struct` (Ruby) | — | Button label + action | `new` |

See the [Window API]({% link api/window.md %}) for `Mouse`, and the [Graphics API]({% link api/graphics.md %}) for `Text`.

## The complete script

The complete program, ready to copy into `examples/subsystems/gui_menus/gui_menus.rb` and run.

{% example ruby examples/subsystems/gui_menus/gui_menus.rb %}

{: .note }
> Immediate mode composes naturally with the [Triggers, Events & ECS]({% link book/ecs.md %}) recipe: both keep state as plain data and logic in the loop. For a logo animation before the menu, see [Logo & Splash]({% link book/gui-splash.md %}).
