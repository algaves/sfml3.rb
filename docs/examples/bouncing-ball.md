---
layout: default
title: 2D Bouncing Ball
parent: Examples
nav_order: 1
---

# Example: 2D Bouncing Ball Physics

A complete, standalone script demonstrating collision response against window borders using `SF::Graphics::CircleShape` and `SF::Graphics::Vector2f`.

```ruby
require 'sfml'

WINDOW_WIDTH  = 800
WINDOW_HEIGHT = 600
RADIUS        = 25.0

mode = SF::VideoMode.new(WINDOW_WIDTH, WINDOW_HEIGHT)

SF::Window::Window.open(mode, 'Example - Bouncing Ball') do |window|
  window.framerate_limit = 60

  shape = SF::Graphics::CircleShape.new(RADIUS)
  shape.fill_color = SF::Graphics::Color::Cyan
  shape.position = SF::Graphics::Vector2f.new(100.0, 100.0)

  velocity = SF::Graphics::Vector2f.new(200.0, 150.0)
  clock = SF::System::Clock.new

  while window.open?
    dt = clock.restart!.as_seconds

    window.poll_events! do |event|
      window.close! if event.is_a?(SF::Window::Event::Closed)
    end

    pos = shape.position
    next_x = pos.x + (velocity.x * dt)
    next_y = pos.y + (velocity.y * dt)

    if next_x <= 0 || next_x + (RADIUS * 2) >= WINDOW_WIDTH
      velocity = SF::Graphics::Vector2f.new(-velocity.x, velocity.y)
    end

    if next_y <= 0 || next_y + (RADIUS * 2) >= WINDOW_HEIGHT
      velocity = SF::Graphics::Vector2f.new(velocity.x, -velocity.y)
    end

    shape.position = SF::Graphics::Vector2f.new(
      pos.x + (velocity.x * dt),
      pos.y + (velocity.y * dt)
    )

    window.render!(clear_color: SF::Graphics::Color.new(20, 20, 30)) do |target|
      target.draw(shape)
    end
  end
end
```
