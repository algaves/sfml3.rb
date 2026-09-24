# frozen_string_literal: true

# Rubyesque (Matz-like): audio. Playback reads as a sentence once the mutators are banged and
# the status is a predicate -- `music.play! if music.stopped?` -- and
# `SoundBufferRecorder.record!` captures into a block and returns the buffer.
# P toggles the sound, S stops it, M starts/stops the looping music, R replays
# the captured buffer, escape quits.
#
# Run from the repository root with a display (and a microphone for the capture):
#   bundle exec ruby -Ilib examples/rubyesque/audio/audio.rb
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

def sound(name, volume: 60)
  buffer = SoundBuffer.from_file(File.join(ASSETS, "#{name}.wav"))
  Sound.new(buffer).tap { |sound| sound.volume = volume }
end

beep = sound('beep')
beep.looping = true

music = Music.from_file(File.join(ASSETS, 'beep.wav'))
music.looping = true
music.volume = 40

# `SoundBufferRecorder.record!(...) { |recorder| ... }` starts the recorder,
# runs the block, stops the recorder even if the block raises, and returns the
# captured SoundBuffer. Recording needs a capture device, so it is guarded.
captured =
  begin
    if SoundRecorder.available? && SoundRecorder.default_device
      buffer = SoundBufferRecorder.record!(sample_rate: 44_100, channel_count: 1) do
        SF.sleep!(SF::System::Time.seconds(0.3))
      end
      recording_note = format('captured %.2fs with record! -- R replays it', buffer.duration.as_seconds)
      Sound.new(buffer)
    else
      recording_note = 'no capture device, record! skipped'
      nil
    end
  rescue StandardError => e
    recording_note = "record! unavailable here (#{e.class})"
    nil
  end

window = Window.new(VideoMode.new(680, 320, 32), 'SFML rubyesque: audio')
window.frame_rate = 60

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      case event.code
      when :escape then window.close!
      when :p then beep.playing? ? beep.pause! : beep.play!
      when :s then beep.stop!
      when :m then music.stopped? ? music.play! : music.stop!
      when :r then captured&.play!
      end
    end
  end

  window.render!(clear_color: [22, 26, 34, 255]) do |target|
    target.draw(text(
                  "beep   playing? #{beep.playing?}  paused? #{beep.paused?}  stopped? #{beep.stopped?}\n" \
                  "music  playing? #{music.playing?}  paused? #{music.paused?}  stopped? #{music.stopped?}\n" \
                  "buffer #{recording_note}\n\n" \
                  'P sound play/pause, S sound stop, M music on/off, R replay capture, escape quits',
                  size: 16, position: [20, 24]
                ))
  end

end
