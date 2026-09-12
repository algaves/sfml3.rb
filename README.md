# sfml3.rb

Ruby bindings for SFML.

## Status

Currently based on **SFML 2**, bound through CSFML (the C API). A migration to **SFML 3** is planned.

## Requirements

* Ruby >= 2.5
* CSFML / SFML 2 development libraries: `csfml-graphics`, `csfml-window`, `csfml-system`, `csfml-audio`

## Build and install

```sh
rake
```

Or manually:

```sh
gem build sfml.gemspec
gem install sfml-<version>.gem
```

## Usage

```ruby
require 'sfml'
include SFML

window = Window.new VideoMode.new(640, 480, 32), 'SFML'

while window.is_open?
  while window.poll_event! Event.new
    window.close! if event.type == 'closed'
  end
  window.clear [0.2, 0.3, 0.4, 1.0]
  window.display
end
```

## Roadmap

* Migrate bindings from SFML 2 (CSFML) to SFML 3.

## License

LGPL-2.1