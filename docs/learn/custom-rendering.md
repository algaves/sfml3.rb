---
layout: default
title: Custom Rendering
parent: Learn
nav_order: 4
---

# Tutorial 4: Custom Rendering (Views & Shaders)

## Views (cameras)

```ruby
require 'sfml'

window = SF::Window::Window.new(SF::VideoMode.new(800, 600), 'View')
view = SF::Graphics::View.new(SF::Graphics::FloatRect.new(0, 0, 800, 600))
view.center = SF::Graphics::Vector2f.new(400, 300)
window.view = view
```

## Shaders (GLSL)

```ruby
vertex = <<~GLSL
  void main() {
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_FrontColor = gl_Color;
  }
GLSL
fragment = <<~GLSL
  uniform float u_time;
  void main() {
    gl_FragColor = vec4(sin(u_time), 0.5, 0.8, 1.0);
  }
GLSL

shader = SF::Graphics::Shader.from_memory(vertex, fragment)
shader.set_float('u_time', 0.0)
clock = SF::System::Clock.new

window.render! do |t|
  shader.set_float('u_time', clock.elapsed_time.as_seconds)
  state = SF::Graphics::RenderState.new(shader: shader)
  t.draw(some_shape, state)
end
```

## RenderTexture (offscreen)

```ruby
rt = SF::Graphics::RenderTexture.new([400, 300])
rt.smooth = true
rt.clear([18, 22, 34, 255])
rt.draw(sprite)
rt.display
sprite_rt = SF::Graphics::Sprite.new(rt.texture)
window.draw(sprite_rt)
```
