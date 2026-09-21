# frozen_string_literal: true

# SFML::Joystick: the real-time state of every gamepad. `Joystick.update!`
# refreshes the cached state; `connected?`/`button_count`/`has_axis?` describe
# what is plugged in, `axis_position` and `button_pressed?` read it, and
# `identification` returns the name and vendor/product ids. R rescans (for
# hot-plugging), escape quits. The event queue also reports connection changes.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/joystick.rb
# (headless: SFML_EXAMPLE_FRAMES=120 xvfb-run -a bundle exec ruby -Ilib ...)

require 'sfml'
require_relative '../support'
include SFML

AXES = %i[x y z r u v pov_x pov_y].freeze

def draw_axes(window, joystick, y)
  AXES.each_with_index do |axis, index|
    next unless Joystick.has_axis?(joystick, axis)

    row = y + (index * 22)
    value = Joystick.axis_position(joystick, axis)
    window.draw(ExampleSupport.text(axis.to_s, size: 14, position: [24, row]))
    track = RectangleShape.new([180, 12])
    track.position = [70, row + 3]
    track.fill_color = [40, 44, 56, 255]
    window.draw(track)
    centre = RectangleShape.new([2, 12])
    centre.position = [159, row + 3]
    centre.fill_color = [120, 128, 150, 255]
    window.draw(centre)
    knob = CircleShape.new(5)
    knob.origin = [5, 5]
    knob.position = [160 + (value / 100.0 * 85), row + 9]
    knob.fill_color = value.abs > 5 ? [90, 200, 255, 255] : [120, 128, 150, 255]
    window.draw(knob)
    window.draw(ExampleSupport.text(format('%6.1f', value), size: 13, position: [262, row]))
  end
end

def draw_buttons(window, joystick, y)
  count = Joystick.button_count(joystick)
  window.draw(ExampleSupport.text("buttons (#{count})", size: 14, position: [380, y - 22]))
  count.times do |button|
    column = button % 8
    row = button / 8
    box = RectangleShape.new([26, 26])
    box.position = [380 + (column * 30), y + (row * 30)]
    box.outline_thickness = 1
    box.outline_color = [120, 128, 150, 255]
    box.fill_color = Joystick.button_pressed?(joystick, button) ? [120, 230, 140, 255] : [40, 44, 56, 255]
    window.draw(box)
  end
end

def draw_joystick(window, id)
  info = Joystick.identification(id)
  window.draw(ExampleSupport.text(
                "joystick #{id}: #{info[:name]}\nvendor #{info[:vendor_id]}  product #{info[:product_id]}",
                size: 16, position: [20, 50]
              ))
  draw_axes(window, id, 110)
  draw_buttons(window, id, 110)
end

window = Window.new(VideoMode.new(680, 440, 32), 'SFML joystick')
window.frame_rate = 60
event = Event.new
frame = 0
connected = nil

loop do
  while window.poll_event!(event)
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      window.close! if event.key[:code] == :escape
      connected = nil if event.key[:code] == :r # force a rescan
    when 'joystick-connected', 'joystick-disconnected'
      connected = nil
    end
  end

  Joystick.update!
  connected ||= (0...Joystick::COUNT).find { |id| Joystick.connected?(id) }

  window.clear([22, 26, 34, 255])
  if connected
    draw_joystick(window, connected)
  else
    window.draw(ExampleSupport.text(
                  "No joystick connected.\nPlug one in, then press R to rescan.\n" \
                  "#{Joystick::COUNT} slots, up to #{Joystick::BUTTON_COUNT} buttons " \
                  "and #{Joystick::AXIS_COUNT} axes each.\n\nEscape quits.",
                  size: 18, position: [30, 80]
                ))
  end
  window.display

  frame += 1
  break if ExampleSupport.auto_close?(frame)
end
