---
layout: default
title: Sprite Animation
parent: Examples
nav_order: 2
---

# Example: Sprite Sheet Animation

Demonstrates loading a sprite sheet and animating via `texture_rect`. This mirrors the pattern used in `examples/subsystems/graphics_assets.rb`.

```ruby
require 'sfml'
require_relative '../../examples/support' # from repo root when running

WIDTH = 640
HEIGHT = 480

window = SF::Window::Window.new(SF::VideoMode.new(WIDTH, HEIGHT, 32), 'Sprite Animation')
window.framerate_limit = 60

BIRD_FRAMES = %i[bird_up bird_mid bird_down bird_mid].freeze
frame = 0
clock = SF::System::Clock.new
interval = 0.15

bird = ExampleSupport.sprite(:bird_down, scale: 3)
bird.position = [WIDTH / 2 - 32, HEIGHT / 2]

while window.open?
  dt = clock.get_elapsed_time.as_seconds

  window.poll_events! do |event|
    window.close! if event.is_a?(SF::Window::Event::Closed)
  end

  if dt >= interval
    frame = (frame + 1) % BIRD_FRAMES.size
    bird = ExampleSupport.sprite(BIRD_FRAMES[frame], scale: 3)
    bird.position = [WIDTH / 2 - 32, HEIGHT / 2]
    clock.restart!
  end

  window.render!(clear_color: SF::Graphics::Color.new(20, 40, 60)) do |t|
    t.draw(bird)
  end
end
```

{: .note }
> When viewing from the built docs, adjust the `require_relative` path depending on where you run the script. For standalone copies, replace with your own sprite loading.
