---
layout: default
title: Audio Visualizer
parent: Examples
nav_order: 3
---

# Example: Basic Audio Visualizer

A simple real-time bar visualization using `SF::Audio::SoundBuffer#samples`. Load a mono or stereo sound, read its PCM samples, and draw amplitude bars.

```ruby
require 'sfml'

SAMPLE_COUNT = 128

def bar_color(i)
  h = i.to_f / SAMPLE_COUNT
  r = (Math.sin(h * 6.0) * 127 + 128).to_i
  g = (Math.sin(h * 6.0 + 2.0) * 127 + 128).to_i
  b = (Math.sin(h * 6.0 + 4.0) * 127 + 128).to_i
  SF::Graphics::Color.new(r, g, b, 255)
end

buffer = SF::Audio::SoundBuffer.new('track.ogg') # replace with your audio file
sound = SF::Audio::Sound.new(buffer)
sound.loop = true
sound.play!

window = SF::Window::Window.new(SF::VideoMode.new(800, 600), 'Audio Visualizer')
window.framerate_limit = 60

samples = buffer.samples # Array<Integer>
channels = buffer.channel_count
sample_rate = buffer.sample_rate
total = samples.size / channels

while window.open?
  window.poll_events! { |e| window.close! if e.is_a?(SF::Window::Event::Closed) }

  window.render!(clear_color: SF::Graphics::Color.new(10, 10, 20)) do |t|
    step = total / SAMPLE_COUNT
    SAMPLE_COUNT.times do |i|
      idx = (i * step) * channels
      val = 0
      channels.times { |c| val += samples[idx + c] if idx + c < samples.size }
      amp = (val / channels.to_f) / 32768.0 # -1.0..1.0
      hbar = (amp.abs * 250).clamp(2, 250)
      rect = SF::Graphics::RectangleShape.new([4, hbar])
      rect.position = [50 + i * 5.5, 300 - hbar / 2]
      rect.fill_color = bar_color(i)
      t.draw(rect)
    end
  end
end
```

{: .note }
> For stereo files, samples are interleaved. The example averages across channels to get a mono amplitude per time slice.
