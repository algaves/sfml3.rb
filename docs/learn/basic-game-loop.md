---
layout: default
title: Basic Game Loop
parent: Learn
nav_order: 1
---

# Tutorial 1: Building a Basic Game Loop

In this tutorial you'll learn how to manage time, handle window events, and structure a clean game loop using the Rubyesque APIs.

## Frame delta time

To keep movement frame-rate independent, measure elapsed time between frames with `SF::System::Clock`:

```ruby
require 'sfml'

mode = SF::VideoMode.new(800, 600)
clock = SF::System::Clock.new

SF::Window::Window.open(mode, 'Game Loop Tutorial') do |window|
  window.framerate_limit = 60

  while window.open?
    dt = clock.restart!.as_seconds

    window.poll_events! do |event|
      window.close! if event.is_a?(SF::Window::Event::Closed)
    end

    window.render!(clear_color: SF::Graphics::Color::Black) do |target|
      # Update and draw game entities using dt
    end
  end
end
```

{: .note }
> Headless machines can run examples under `xvfb-run -a` with `SFML_EXAMPLE_FRAMES=120` for smoke tests.
