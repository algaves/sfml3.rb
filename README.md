# sfml3.rb

Ruby bindings for [SFML 3](https://www.sfml-dev.org/), via its C API, [CSFML](https://github.com/SFML/CSFML).

## Status

Bound against **CSFML 3**. `gem install sfml3-rb` downloads and builds SFML 3 and CSFML 3 from source
automatically — neither needs to be installed system-wide. See [TODO.md](TODO.md) for which parts of
the SFML 3 API are ported so far, and [CHANGELOG.md](CHANGELOG.md) for what changed recently.

## Requirements

* Ruby >= 2.5
* A C/C++ toolchain and CMake >= 3.22, to build the vendored SFML 3 and CSFML 3 from source
* On Linux, the X11/udev/OpenGL development headers SFML links against — these can't be bundled.
  For example, on Fedora:

  ```sh
  sudo dnf install cmake gcc-c++ freetype-devel libX11-devel \
    libXrandr-devel libXcursor-devel libXi-devel systemd-devel libglvnd-devel
  ```

  or on Debian/Ubuntu:

  ```sh
  sudo apt-get install cmake build-essential libfreetype-dev libx11-dev \
    libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev
  ```

Already have CSFML 3 installed system-wide? Skip the source build entirely — see below.

## Install

```sh
gem install sfml3-rb
```

This builds SFML 3 and CSFML 3 from source, which takes roughly half a minute. Each installed gem
version builds its own copy; there's no build cache shared across versions.

To link against a system CSFML 3 instead (no download, no build):

```sh
gem install sfml3-rb -- --enable-system-libraries
```

## Usage

```ruby
require 'sfml'
include SFML

window = Window.new VideoMode.new(640, 480, 32), 'SFML'
event = Event.new

while window.is_open?
  while window.poll_event! event
    window.close! if event.type == 'closed'
  end

  window.clear [51, 76, 102, 255] # [r, g, b, a], 0-255
  window.display
end
```

See [`test/hello-world.rb`](test/hello-world.rb) for a fuller example with shapes and transforms.

## Development

```sh
rake ports    # build the vendored SFML 3 + CSFML 3 (once; rake compile does this automatically too)
rake compile  # build the C extension
rake test     # compile, then run the test suite
```

`CMakeLists.txt` is a CLion/IDE convenience build against the same vendored `ports/` prefix — `rake`
is the build of record.

## License

[0BSD](LICENSE.md)
