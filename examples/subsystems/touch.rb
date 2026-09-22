# frozen_string_literal: true

# SF::Window::Touch: multitouch fingers. `Touch.down?(finger)` and
# `Touch.position(finger, window)` read the ten tracked fingers in real time;
# touch-began/moved/ended events carry the same data. Desktops without a
# touchscreen (and touchpads that do not synthesise touch) report nothing, so
# the window stays empty here -- run it on a touch device to see fingers appear.
# Escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/touch.rb
# (headless: SFML_EXAMPLE_FRAMES=120 xvfb-run -a bundle exec ruby -Ilib ...)

require 'sfml'
require_relative '../support'
include SF::Window
include SF::Graphics
include SF::System
include SF::Audio
include SF::Network

FINGERS = (0...10)
COLORS = [[230, 90, 90], [90, 200, 130], [90, 170, 230], [235, 200, 90], [180, 110, 235],
          [235, 140, 90], [90, 210, 210], [210, 90, 170], [150, 200, 90], [150, 150, 235]].freeze

window = Window.new(VideoMode.new(640, 420, 32), 'SFML touch')
window.frame_rate = 60
frame = 0
last = 'no touch events yet'

loop do
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      window.close! if event.code == :escape
    when 'touch-began', 'touch-moved', 'touch-ended'
      touch = event.touch
      last = "#{event.type}  finger #{touch[:finger]} at " \
             "#{touch[:position].x.to_i}, #{touch[:position].y.to_i}"
    end
  end

  window.clear!([22, 26, 34, 255])
  window.draw(ExampleSupport.text(
                "Real-time fingers (Touch.down? / Touch.position)\n" \
                "events: #{last}\nescape quits",
                size: 16, position: [20, 16]
              ))

  FINGERS.each do |finger|
    next unless Touch.down?(finger)

    point = Touch.position(finger, window)
    color = COLORS[finger]
    halo = CircleShape.new(34)
    halo.origin = [34, 34]
    halo.position = [point.x, point.y]
    halo.fill_color = [color[0], color[1], color[2], 60]
    window.draw(halo)

    dot = CircleShape.new(16)
    dot.origin = [16, 16]
    dot.position = [point.x, point.y]
    dot.fill_color = [color[0], color[1], color[2], 235]
    window.draw(dot)

    window.draw(ExampleSupport.text(finger.to_s, size: 15,
                                                 position: [point.x - 5, point.y - 9],
                                                 color: [20, 20, 28, 255]))
    window.draw(ExampleSupport.text("finger #{finger}", size: 13,
                                                        position: [point.x + 38, point.y - 8]))
  end

  window.display!
  frame += 1
  break if ExampleSupport.auto_close?(frame)
end
