# frozen_string_literal: true

# Window styles and states. Window.new takes a style as a single symbol, an
# array of flags, or the raw integer bitmask, plus a :windowed / :fullscreen
# state. The default is :titlebar | :resize | :close. Press 1-6 to rebuild the
# window with a different combination; the minimum/maximum size and the title
# carry over.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/style.rb
# (headless: SFML_EXAMPLE_FRAMES=120 xvfb-run -a bundle exec ruby -Ilib ...)
# Escape or the close button quits.

require 'sfml'
require_relative '../support'
include SF::Window
include SF::Graphics
include SF::System
include SF::Audio
include SF::Network

STYLES = [
  [:default, :windowed, 'default  =  titlebar + resize + close'],
  [:none, :windowed, 'none  =  undecorated'],
  [:titlebar, :windowed, 'titlebar only'],
  [%i[titlebar resize], :windowed, 'titlebar + resize'],
  [%i[resize close], :windowed, 'resize + close'],
  [:default, :fullscreen, 'default style, :fullscreen state']
].freeze

def build_window(style, state, label)
  Window.new(VideoMode.new(720, 480, 32), "SFML style: #{label}", style, state).tap do |window|
    window.frame_rate = 60
    window.minimum_size = [320, 240]
    window.maximum_size = [1280, 720]
  end
end

index = 0
window = build_window(*STYLES[index])
frame = 0

loop do
  rebuild = nil

  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      code = event.code
      window.close! if code == :escape
      number = code.to_s[/\Anum([1-6])\z/, 1]
      rebuild = number.to_i - 1 if number
    end
  end

  # Rebuilding is deferred until the event queue is drained, so the old window
  # is not closed while it is still being polled.
  if rebuild
    index = rebuild
    window.close!
    window = build_window(*STYLES[index])
  end

  info = ExampleSupport.text(
    "style #{index + 1}/#{STYLES.size}: #{STYLES[index][2]}\n" \
    "size #{window.size.x.to_i}x#{window.size.y.to_i}, " \
    "min 320x240, max 1280x720\n" \
    'press 1-6 to change, escape to quit',
    size: 17
  )

  window.clear!(STYLES[index][1] == :fullscreen ? [40, 30, 60, 255] : [26, 30, 40, 255])
  window.draw(info)
  window.display!

  frame += 1
  break if ExampleSupport.auto_close?(frame)
end
