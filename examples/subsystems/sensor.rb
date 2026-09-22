# frozen_string_literal: true

# SF::Window::Sensor: the device's hardware sensors. `available?` reports whether a
# type exists here, `set_enabled` turns it on (some sensors, notably the
# gyroscope, report nothing until enabled) and `value` returns a Vector3.
# Desktops usually expose none of these, so the example renders an
# "unavailable" panel for each rather than failing. Escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/sensor.rb
# (headless: SFML_EXAMPLE_FRAMES=120 xvfb-run -a bundle exec ruby -Ilib ...)

require 'sfml'
require_relative '../support'
include SF::Window
include SF::Graphics
include SF::System
include SF::Audio
include SF::Network

TYPES = %i[accelerometer gyroscope magnetometer gravity user_acceleration orientation].freeze

def draw_sensor(window, type, index)
  column = index % 2
  row = index / 2
  x = 24 + (column * 330)
  y = 90 + (row * 110)

  available = Sensor.available?(type)
  Sensor.set_enabled(type, true) if available
  value = available ? Sensor.value(type) : Vector3.new(0, 0, 0)

  window.draw(ExampleSupport.text(
                "#{type}  #{available ? 'available' : 'unavailable'}",
                size: 16, position: [x, y], color: available ? [235, 235, 245, 255] : [150, 155, 170, 255]
              ))
  return unless available

  %i[x y z].each_with_index do |axis, axis_index|
    amount = value.public_send(axis)
    track = RectangleShape.new([200, 10])
    track.position = [x + 40, y + 24 + (axis_index * 18)]
    track.fill_color = [40, 44, 56, 255]
    window.draw(track)
    marker = RectangleShape.new([10, 10])
    marker.position = [x + 140 + (amount.clamp(-1.0, 1.0) * 90), y + 24 + (axis_index * 18)]
    marker.fill_color = [90, 200, 255, 255]
    window.draw(marker)
    label = "#{axis} #{amount.round(2).to_s.rjust(6)}"
    window.draw(ExampleSupport.text(label, size: 13, position: [x + 250, y + 21 + (axis_index * 18)]))
  end
end

window = Window.new(VideoMode.new(700, 440, 32), 'SFML sensor')
window.frame_rate = 60
frame = 0
last_event = 'move the device (or watch for "unavailable")'

loop do
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      window.close! if event.code == :escape
    when 'sensor-changed'
      change = event.sensor
      last_event = "sensor-changed #{change[:type]} -> #{change[:value]}"
    end
  end

  window.clear!([22, 26, 34, 255])
  window.draw(ExampleSupport.text(
                "SF::Window::Sensor -- availability and values (#{last_event})\n" \
                'sensors are enabled on entry; escape quits',
                size: 16, position: [24, 16]
              ))
  TYPES.each_with_index { |type, index| draw_sensor(window, type, index) }
  window.display!

  frame += 1
  break if ExampleSupport.auto_close?(frame)
end
