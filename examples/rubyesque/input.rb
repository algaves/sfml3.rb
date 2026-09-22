# frozen_string_literal: true

# Rubyesque (Matz-like): input devices. `Keyboard.key_pressed?` reads a logical key,
# `Joystick.axis?` asks whether a pad has an axis, `Sensor.enable!`/`disable!`
# turn a hardware sensor on and off, and `Touch.position(..., relative_to:)`
# reports a finger in window coordinates. G toggles the gyroscope, escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/rubyesque/input.rb
# (headless: SFML_EXAMPLE_FRAMES=120 xvfb-run -a bundle exec ruby -Ilib ...)

require 'sfml'
require_relative '../support'
include SF::Window
include SF::Graphics
include SF::System
include SF::Audio
include SF::Network

KEYS = %i[w a s d Space].freeze
gyroscope = false

window = Window.new(VideoMode.new(720, 420, 32), 'SFML rubyesque: input')
window.frame_rate = 60
frame = 0

loop do
  window.poll_events! do |event|
    window.close! if event.closed?
    window.close! if event.key_pressed? && event.code == :escape
    if event.key_pressed? && event.code == :g
      gyroscope = !gyroscope
      # enable!/disable! read better than set_enabled(type, true/false).
      gyroscope ? Sensor.enable!(:gyroscope) : Sensor.disable!(:gyroscope)
    end
  end

  pressed = KEYS.select { |key| Keyboard.key_pressed?(key) }

  lines = ["Keyboard.key_pressed?: #{pressed.empty? ? '(none)' : pressed.join(' ')}"]

  Joystick.update!
  pad = (0...Joystick::COUNT).find { |id| Joystick.connected?(id) }
  if pad
    axis = %i[x y].select { |name| Joystick.axis?(pad, name) }
    lines << "joystick #{pad}: axes #{axis.join('/')}, " \
             "position #{Joystick.axis_position(pad, :x).round}, " \
             "#{Joystick.axis_position(pad, :y).round}"
  else
    lines << 'joystick: none connected'
  end

  if Sensor.available?(:gyroscope)
    value = gyroscope ? Sensor.value(:gyroscope) : Vector3.new(0, 0, 0)
    lines << "gyroscope (G toggles): enabled #{gyroscope}, " \
             "value #{value.x.round(2)}, #{value.y.round(2)}, #{value.z.round(2)}"
  else
    lines << 'gyroscope: unavailable on this machine'
  end

  if Touch.down?(0)
    # `relative_to:` names the window the coordinates are reported in, instead
    # of passing it positionally.
    point = Touch.position(0, relative_to: window)
    lines << "touch finger 0 at #{point.x.to_i}, #{point.y.to_i}"
  end

  window.render!(clear_color: [22, 26, 34, 255]) do |target|
    target.draw(ExampleSupport.text(
                  "#{lines.join("\n")}\n\nWASD/Space, G gyroscope, escape quits",
                  size: 16, position: [20, 20]
                ))
  end

  frame += 1
  break if ExampleSupport.auto_close?(frame)
end
