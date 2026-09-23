---
layout: default
title: Playing Audio
parent: Learn
nav_order: 3
---

# Tutorial 3: Playing Audio

## Sound effects

```ruby
require 'sfml'

buffer = SF::Audio::SoundBuffer.new('beep.wav')
sound = SF::Audio::Sound.new(buffer)
sound.volume = 50
sound.play!
```

## Music streaming

```ruby
music = SF::Audio::Music.new
music.open_from_file('track.ogg')
music.loop = true
music.play!
```

## Listener (spatial audio)

```ruby
SF::Audio::Listener.set_position(0, 0, 0)
SF::Audio::Listener.set_direction(0, 0, -1)
SF::Audio::Listener.set_global_volume(100)
```

## Audio devices

```ruby
devices = SF::Audio::SoundRecorder.available_devices
default = SF::Audio::SoundRecorder.default_device
SF::Audio::SoundRecorder.device = devices.first if devices.any?
```
