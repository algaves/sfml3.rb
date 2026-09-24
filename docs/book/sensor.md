---
layout: default
title: Sensor
parent: "Part III — Intermediate"
grand_parent: Book
nav_order: 3
---

# Recipe: Sensor

`SF::Window::Sensor`: the device's hardware sensors. `available?` reports whether a type exists, `enable!` turns it on (some sensors report nothing until enabled) and `value` returns a `Vector3`. Desktops usually expose none, so the example renders an "unavailable" panel rather than failing.

> **Intermediate**

## Goal

A 700×440 window with one panel per sensor type (`accelerometer`, `gyroscope`, `magnetometer`, `gravity`, `user_acceleration`, `orientation`) showing whether it is available and, when it is, a bar per axis with a live marker and value. A `'sensor-changed'` event line reports device motion. Escape quits.

## How the code works

### 1. Setup helpers and sensor types

`Font.from_file` and the `text` helper build every label, and `TYPES` lists the six sensors the example asks about. Keeping the list in one place lets the draw loop iterate over the same set the panels use.

{% example ruby examples/subsystems/sensor/sensor.rb 21-29 %}

### 2. Ask before reading

`Sensor.available?` decides whether a panel is live; only then is `Sensor.set_enabled(type, true)` called, because some sensors report zeros until enabled. An unavailable type substitutes a zero `Vector3` rather than reading meaningless data.

{% example ruby examples/subsystems/sensor/sensor.rb 37-39 %}

### 3. Panel header and availability

Each panel is placed in a two-column grid at an x and y derived from its index. The header prints the type name and whether it is available, colouring unavailable panels grey and returning early so nothing else is drawn for them.

{% example ruby examples/subsystems/sensor/sensor.rb 41-45 %}

### 4. One bar per axis

For `x`, `y` and `z`, `Sensor.value(type).public_send(axis)` reads the component, which is clamped to `-1.0`–`1.0`, mapped onto a dark track as a moving marker, and printed rounded beside it.

{% example ruby examples/subsystems/sensor/sensor.rb 47-59 %}

### 5. Open the window and watch the event stream

The window runs at 60 fps and `last_event` seeds the header. Escape closes it, and a `'sensor-changed'` event carries `event.sensor`, a hash of `:type` and `:value`, which is formatted into `last_event`.

{% example ruby examples/subsystems/sensor/sensor.rb 62-77 %}

### 6. Draw the header and every panel

`clear!` erases the previous frame, the header shows the latest event, and `TYPES.each_with_index` draws one panel per type before `display!` presents the frame.

{% example ruby examples/subsystems/sensor/sensor.rb 79-86 %}

## Ingredients

| Class | Subsystem | Role | Key methods |
| --- | --- | --- | --- |
| `Sensor` | Window | Sensor availability, enable, value | `available?`, `set_enabled`, `enable!`, `value` |
| `Vector3` | System | The three-axis reading | `new`, `x`, `y`, `z` |
| `Event` | Window | `sensor-changed` notifications | `type`, `sensor` |
| `RectangleShape` | Graphics | Axis tracks and markers | `new`, `position=`, `fill_color=` |
| `Text` / `Font` | Graphics | Type names, axis labels, values | `Font.from_file`, `Text.new`, `color:` |
| `Window` | Window | Surface and event queue | `new`, `poll_events!`, `clear!`, `draw`, `display!` |

See the [Input API]({% link api/input.md %}) and [System API]({% link api/system.md %}) for the full signature of each.

## The complete script

The complete program, ready to copy into `examples/subsystems/sensor/sensor.rb` and run.

{% example ruby examples/subsystems/sensor/sensor.rb %}

{: .note }
> Always `available?` first and `enable!` before reading `value`. Types are `:accelerometer`, `:gyroscope`, `:magnetometer`, `:gravity`, `:user_acceleration` and `:orientation`.
