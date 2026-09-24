# frozen_string_literal: true

# Audio devices, through the closest surface CSFML exposes. SFML has no
# `PlaybackDevice` class (there is nothing to bind for it in CSFML 3.0.0), but
# SF::Audio::SoundRecorder enumerates the capture devices and lets you pick one:
# `SoundRecorder.available_devices` and `default_device` list them,
# `SoundRecorder#device=` selects one, and a SoundBufferRecorder captures a few
# seconds that are then played back through a Sound. Playback itself lives on
# Sound / Music (see listener.rb and the games). R records, 1-9 pick a device,
# escape quits.
#
# Run from the repository root with a display and a microphone:
#   bundle exec ruby -Ilib examples/subsystems/audio_devices/audio_devices.rb
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

RECORD_SECONDS = 3

window = Window.new(VideoMode.new(680, 440, 32), 'SFML audio devices')
window.frame_rate = 60

recorder = SoundBufferRecorder.new
recorder.channel_count = 1
recorder.device = SoundRecorder.default_device if SoundRecorder.default_device

clock = Clock.new
state = 'idle'
recorded = nil

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      key = event.code
      window.close! if key == :escape
      if key == :r && state != 'recording'
        state = recorder.start(44_100) ? 'recording' : 'could not start'
        clock.restart!
      end
      number = key.to_s[/\Anum([1-9])\z/, 1]
      if number
        device = SoundRecorder.available_devices[number.to_i - 1]
        recorder.device = device if device
      end
    end
  end

  if state == 'recording' && clock.elapsed_time >= SF::System::Time.seconds(RECORD_SECONDS)
    recorder.stop
    recorded = recorder.buffer
    playback = Sound.new(recorded)
    playback.play!
    state = 'playing back'
  end

  devices = SoundRecorder.available? ? SoundRecorder.available_devices : []
  lines = ["SoundRecorder.available? -> #{SoundRecorder.available?}",
           "default device: #{SoundRecorder.default_device.inspect}",
           "selected device: #{recorder.device.inspect}",
           "recordings: #{recorded ? "#{recorded.duration} of audio" : '(none)'}",
           "state: #{state}",
           '',
           'devices:']
  devices.each_with_index { |device, index| lines << "  #{index + 1}. #{device}" }
  lines << '  (none reported)' if devices.empty?

  window.clear!([22, 26, 34, 255])
  window.draw(text(lines.join("\n"), size: 16))
  window.draw(text(
                'R records 3 s then plays it back; press 1-9 to pick a device; escape quits.',
                size: 14, position: [20, 390]
              ))
  window.display!

end
