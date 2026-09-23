---
layout: default
title: Handling Input
parent: Learn
nav_order: 2
---

# Tutorial 2: Handling Input

`sfml3-rb` provides both event polling (`poll_events!`) and direct state queries for low-latency input.

## Keyboard state

```ruby
require 'sfml'

SF::Window::Window.open(SF::VideoMode.new(640, 480), 'Input') do |window|
  window.framerate_limit = 60
  while window.open?
    window.poll_events! { |e| window.close! if e.is_a?(SF::Window::Event::Closed) }

    if SF::Window::Keyboard.key_pressed?(:space)
      # Space held
    end
  end
end
```

## Mouse

```ruby
pos = SF::Window::Mouse.get_position(window)
desktop = SF::Window::Mouse.get_position
```

## Joystick and Touch

```ruby
if SF::Window::Joystick.is_connected?(0)
  x = SF::Window::Joystick.get_axis_position(0, :x)
end
if SF::Window::Touch.is_down?(0)
  p = SF::Window::Touch.get_position(0, window)
end
```

## Sensors

```ruby
SF::Window::Sensor.enable(:accelerometer)
val = SF::Window::Sensor.get_value(:accelerometer)
```
