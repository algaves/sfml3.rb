---
layout: default
title: Quick Start
nav_order: 2
---

# Quick Start

```ruby
require 'sfml'
include SF::Window
include SF::Graphics
include SF::System
include SF::Audio
include SF::Network

window = Window.new VideoMode[640, 480, 32], 'SFML'

while window.open?
  window.poll_events! do |event|
    window.close! if event.closed?
  end

  window.clear! [51, 76, 102, 255] # [r, g, b, a], 0-255
  window.display!
end
```

The primary spellings (`open?`, `clear!`, `display!`) are the Rubyesque (Matz-like) layer. The pre-existing names still work but warn when deprecated.
