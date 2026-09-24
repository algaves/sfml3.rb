# frozen_string_literal: true

# A real-time bar chart of the PCM samples in the bundled beep.wav, looped.
# `SoundBuffer#samples` returns the raw 16-bit integers; each bar averages a
# horizontal slice (across channels) and its height is the amplitude. Escape
# quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/audio_visualizer/audio_visualizer.rb
# (headless: prefix `xvfb-run -a`).

require 'sfml'
include SF::Window
include SF::Graphics
include SF::System
include SF::Audio

# --- self-contained helpers -------------------------------------------------
ASSETS = File.expand_path('assets', __dir__)

FONT = Font.from_file(File.join(ASSETS, 'LiberationSans-Regular.ttf'))
def text(string, size: 18, position: [12, 8], color: [235, 235, 245, 255])
  label = Text.new(FONT, string, size)
  label.fill_color = color
  label.position = position
  label
end

BARS = 128
WIDTH = 800
HEIGHT = 600

def bar_color(index)
  hue = index.to_f / BARS
  r = ((Math.sin(hue * 6.0) * 127) + 128).to_i
  g = ((Math.sin((hue * 6.0) + 2.0) * 127) + 128).to_i
  b = ((Math.sin((hue * 6.0) + 4.0) * 127) + 128).to_i
  [r, g, b, 255]
end

buffer = SoundBuffer.from_file(File.join(ASSETS, 'beep.wav'))
sound = Sound.new(buffer)
sound.loop = true
sound.play!

samples = buffer.samples
channels = buffer.channel_count
total = samples.length / channels
step = total / BARS

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML audio visualizer')
window.frame_rate = 60

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      window.close! if event.code == :escape
    end
  end

  window.clear!([10, 10, 20, 255])

  BARS.times do |index|
    start = (index * step) * channels
    value = 0
    channels.times do |channel|
      value += samples[start + channel] if start + channel < samples.length
    end
    amplitude = (value / channels.to_f) / 32_768.0
    height = (amplitude.abs * 250).clamp(2, 250)
    bar = RectangleShape.new([4, height])
    bar.position = [50 + (index * 5.5), (HEIGHT / 2) - (height / 2)]
    bar.fill_color = bar_color(index)
    window.draw(bar)
  end

  window.draw(text('amplitudes from SoundBuffer#samples -- escape quits',
                   size: 14, position: [40, HEIGHT - 30]))
  window.display!

end
