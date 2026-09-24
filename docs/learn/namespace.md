---
layout: default
title: The SF Namespace
parent: Learn
nav_order: 0
---

# Tutorial 0: The `SF` Namespace and Its Submodules

Everything `sfml3-rb` adds to Ruby lives under one top-level module, `SF`. Under it sit five subsystem modules that mirror SFML's own namespaces. Knowing which module a class belongs to is enough to find it in the [API Reference]({% link api/index.md %}) and to decide how to write its name.

## One require, five modules

`require 'sfml'` loads the whole binding plus the Rubyesque layer, which defines:

| Module | Holds | Reach for it when |
| --- | --- | --- |
| `SF::System` | `Vector2`, `Vector3`, `Time`, `Clock`, `Sleep`, `InputStream`, `Buffer` | timing, vectors, IO |
| `SF::Graphics` | `RenderWindow`, `RenderTexture`, `RenderTarget`, `RenderState`, `Sprite`, `Texture`, `Image`, shapes (`Shape`, `CircleShape`, `RectangleShape`, `ConvexShape`), `Text`/`Font`/`Glyph`, `Vertex`/`VertexArray`/`VertexBuffer`, `View`, `Transform`/`Transformable`/`Drawable`, `Shader`, `Color`, `Rect` | anything drawn |
| `SF::Window` | `WindowBase`, `Window`, `VideoMode`, `Style`, `State`, `Event`, `Context`/`ContextSettings`, `Cursor`, `Vulkan`, and the input modules `Keyboard`, `Mouse`, `Joystick`, `Touch`, `Sensor`, `Clipboard` | the OS window and input |
| `SF::Audio` | `Sound`, `SoundBuffer`, `Music`, `SoundStream`, `SoundSource`, `SoundRecorder`, `SoundBufferRecorder`, `Listener`, `SoundSourceCone` | playback and capture |
| `SF::Network` | `IpAddress`, `Packet`, `TcpSocket`, `TcpListener`, `UdpSocket`, `SocketSelector`, `Http`/`HttpRequest`/`HttpResponse`, `Ftp` | sockets and protocols |

`Window` and `RenderWindow` are easy to mix up: both are in the window hierarchy, but `RenderWindow` lives in `SF::Graphics` because it adds the `RenderTarget` drawing surface. The [Window]({% link book/window-classes.md %}) example shows all three side by side.

## Fully qualified or mixed in

Any class can be named by its full path, but the examples `include` the subsystem modules so the names read like the C++ API:

```ruby
require 'sfml'

# Fully qualified — clear in libraries and long files:
window = SF::Graphics::RenderWindow.new(SF::Window::VideoMode.new(640, 480, 32), 'title')
color  = SF::Graphics::Color.new(230, 90, 90)

# Or mix the subsystem in and drop the prefix — what most examples do:
include SF::Window
include SF::Graphics

window = RenderWindow.new(VideoMode.new(640, 480, 32), 'title')
color  = Color.new(230, 90, 90)
```

## The SFML alias

Code written before the namespace was renamed keeps working: `SFML` is a deprecated alias of `SF`, so `SFML::Audio::Sound` and `SF::Audio::Sound` are the same class. New code should use `SF`.

## Where to go next

- [API Reference]({% link api/index.md %}) — one page per subsystem, listing every class.
- [Examples]({% link book/index.md %}) — runnable, categorized demos.
- The tour script below prints the live contents of the root module and every subsystem, including the [`SFML` alias check](#the-sfml-alias).

## The complete script

{% example ruby examples/namespace_tour/namespace_tour.rb %}
