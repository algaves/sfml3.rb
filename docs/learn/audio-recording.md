---
layout: default
title: Recording & Spatial Audio
parent: Learn
nav_order: 7
---

# Tutorial 7: Recording & Spatial Audio

Beyond play/stop: capture from the mic, position sounds in 3D, and hook per-source DSP.

## Capture to a buffer

```ruby
require 'sfml'

puts SF::Audio::SoundRecorder.available_devices.inspect
buffer = SF::Audio::SoundBufferRecorder.record!(sample_rate: 44_100) do |_rec|
  SF.sleep!(2.0) # capture while the block runs
end
SF::Audio::Sound.new(buffer).play!
```

Manual form (`start` returns bool — check it):

```ruby
rec = SF::Audio::SoundBufferRecorder.new
if rec.start(44_100)
  SF.sleep!(SF::System::Time.seconds(2))
  rec.stop
  SF::Audio::Sound.new(rec.buffer).play!
end
```

Custom processing — subclass `SoundRecorder` and implement `#on_process` (falsy stops):

```ruby
class Level < SF::Audio::SoundRecorder
  def on_process(samples)
    puts "peak: #{samples.map(&:abs).max}"
    true
  end
end
```

Callbacks arrive on SFML's capture thread — keep them fast, never raise through them.

## Custom streams

Subclass `SoundStream`, implement `#on_get_data` (chunk or `nil` for end):

```ruby
class Sine < SF::Audio::SoundStream
  def initialize
    super(1, 44_100)
    @phase = 0.0
  end

  def on_get_data
    Array.new(4_410) do
      @phase += 440.0 * 2 * Math::PI / 44_100
      (Math.sin(@phase) * 20_000).to_i
    end
  end
end
Sine.new.play!
```

## 3D positioning

```ruby
SF::Audio::Listener.position = SF::System::Vector3[0, 0, 0]
SF::Audio::Listener.direction = SF::System::Vector3[0, 0, -1]
sound.position = SF::System::Vector3[5, 0, 0] # right ear
sound.spatialization_enabled = true
sound.min_distance = 4
sound.attenuation = 6.0
sound.play!
```

Directional sources add a cone (`SoundSourceCone.new(inner, outer, gain)`); no effect without `spatialization_enabled` + a `direction`.

## DSP hook

```ruby
sound.effect_processor = proc do |frames, channels|
  frames.map { |f| f.map { |s| s * 0.5 } }
end
sound.effect_processor = nil # clear
```

Fast, non-blocking, never `stop` the source inside. Pool is 32 slots process-wide.

## Worked examples

- [Audio Visualizer]({% link book/audio-visualizer.md %}) — raw PCM through `SoundBuffer#samples`.
- [API: Audio]({% link api/audio.md %}) — the full `SoundSource` control table, recorder and listener.
- The `subsystems/audio_devices` example — a 3-second `SoundBufferRecorder` capture played back.
- The `subsystems/listener` example — a spatialized, moving `Sound` with a cone.
