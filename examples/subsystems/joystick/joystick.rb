# frozen_string_literal: true

# SF::Window::Joystick: the real-time state of every gamepad. `Joystick.update!`
# refreshes the cached state; `connected?`/`button_count`/`axis?` describe
# what is plugged in, `axis_position` and `button_pressed?` read it, and
# `identification` returns the name and vendor/product ids. R rescans (for
# hot-plugging), escape quits. The event queue also reports connection changes.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/joystick/joystick.rb
# (headless: prefix `xvfb-run -a`).

require 'sfml'
include SF::Window
include SF::Graphics
include SF::System

# --- self-contained helpers -------------------------------------------------
ASSETS = File.expand_path('assets', __dir__)

FONT = Font.from_file(File.join(ASSETS, 'LiberationSans-Regular.ttf'))
def text(string, size: 18, position: [12, 8], color: [235, 235, 245, 255])
  label = Text.new(FONT, string, size)
  label.fill_color = color
  label.position = position
  label
end

AXES = %i[x y z r u v pov_x pov_y].freeze

def draw_axes(window, joystick, y)
  AXES.each_with_index do |axis, index|
    next unless Joystick.axis?(joystick, axis)

    row = y + (index * 22)
    value = Joystick.axis_position(joystick, axis)
    window.draw(text(axis.to_s, size: 14, position: [24, row]))
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
    window.draw(text(format('%6.1f', value), size: 13, position: [262, row]))
  end
end

def draw_buttons(window, joystick, y)
  count = Joystick.button_count(joystick)
  window.draw(text("buttons (#{count})", size: 14, position: [380, y - 22]))
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
  window.draw(text(
                "joystick #{id}: #{info[:name]}\nvendor #{info[:vendor_id]}  product #{info[:product_id]}",
                size: 16, position: [20, 50]
              ))
  draw_axes(window, id, 110)
  draw_buttons(window, id, 110)
end

window = Window.new(VideoMode.new(680, 440, 32), 'SFML joystick')
window.frame_rate = 60
connected = nil

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      window.close! if event.code == :escape
      connected = nil if event.code == :r # force a rescan
    when 'joystick-connected', 'joystick-disconnected'
      connected = nil
    end
  end

  Joystick.update!
  connected ||= (0...Joystick::COUNT).find { |id| Joystick.connected?(id) }

  window.clear!([22, 26, 34, 255])
  if connected
    draw_joystick(window, connected)
  else
    window.draw(text(
                  "No joystick connected.\nPlug one in, then press R to rescan.\n" \
                  "#{Joystick::COUNT} slots, up to #{Joystick::BUTTON_COUNT} buttons " \
                  "and #{Joystick::AXIS_COUNT} axes each.\n\nEscape quits.",
                  size: 18, position: [30, 80]
                ))
  end
  window.display!

end
