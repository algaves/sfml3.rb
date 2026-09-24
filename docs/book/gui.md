---
layout: default
title: GUI
parent: "Part IV — Advanced"
grand_parent: Book
nav_order: 6
has_children: true
permalink: /book/gui
---

# GUI

A game's interface is state you draw and input you interpret. SFML gives you no widget toolkit, so the simplest approach is **immediate mode**: rather than building button objects, the loop describes the interface from scratch every frame — measure the mouse, draw the result, run an action on click. Nothing persists but a few variables, and the screen is always an exact function of the current state.

The pieces are ordinary `RectangleShape`s (panels and tracks), `Text` for labels, `Mouse.position(window)` and `Rect#contains?` for hit-testing, and the event queue for clicks and keys. Two recipes cover the common cases:

1. **[Menus & HUD]({% link book/gui-menus.md %})** — a menu, an options screen with a slider, and an in-game HUD.
2. **[Logo & Splash]({% link book/gui-splash.md %})** — a timed logo animation that fades into the menu.

{: .note }
> Immediate mode composes naturally with the [Triggers, Events & ECS]({% link book/ecs.md %}) recipe: both keep state as plain data and logic in the loop. See the [Window API]({% link api/window.md %}) for `Mouse` and the [Graphics API]({% link api/graphics.md %}) for `Text`, `Sprite` and `RectangleShape`.
