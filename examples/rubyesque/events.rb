# frozen_string_literal: true

# Rubyesque (Matz-like): events. Every event kind has a predicate (`event.closed?`,
# `event.key_pressed?`, `event.mouse_moved?`, ...), so no `case event.type`
# strings are needed, and `event.code` is the `event.key[:code]` shortcut.
# Move the mouse, click, scroll, type or plug in a gamepad and the HUD shows
# the event through its predicate. Escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/rubyesque/events.rb
# (headless: SFML_EXAMPLE_FRAMES=120 xvfb-run -a bundle exec ruby -Ilib ...)

require 'sfml'
require_relative '../support'
include SF::Window
include SF::Graphics
include SF::System
include SF::Audio
include SF::Network

# Predicates are plain methods, so they compose in any control flow. This one
# turns an event into a sentence; the payload hashes (`event.key`,
# `event.mouse_button`, `event.touch`, ...) are the exact same ones as before.
def describe(event)
  return 'closed' if event.closed?
  return "resized to #{event.size.x.to_i}x#{event.size.y.to_i}" if event.resized?
  return 'gained focus' if event.gained_focus?
  return 'lost focus' if event.lost_focus?
  return "text entered \"#{event.text}\"" if event.text_entered?
  return "key #{event.code} pressed (code is event.key[:code])" if event.key_pressed?
  return "key #{event.code} released" if event.key_released?
  return "wheel scrolled #{event.mouse_wheel_scroll[:delta]}" if event.mouse_wheel_scrolled?
  return "mouse button #{event.mouse_button[:button]} pressed" if event.mouse_button_pressed?
  return "mouse button #{event.mouse_button[:button]} released" if event.mouse_button_released?
  return "mouse moved to #{event.mouse_move.x.to_i}, #{event.mouse_move.y.to_i}" if event.mouse_moved?
  return 'mouse entered the window' if event.mouse_entered?
  return 'mouse left the window' if event.mouse_left?
  return "joystick #{event.joystick_connect[:joystick_id]} plugged in" if event.joystick_connected?
  return 'joystick unplugged' if event.joystick_disconnected?
  return 'joystick moved' if event.joystick_moved?
  return 'joystick button pressed' if event.joystick_button_pressed?
  return "touch began, finger #{event.touch[:finger]}" if event.touch_began?
  return "touch moved, finger #{event.touch[:finger]}" if event.touch_moved?
  return "touch ended, finger #{event.touch[:finger]}" if event.touch_ended?
  return "sensor changed: #{event.sensor[:type]}" if event.sensor_changed?

  "#{event.type} (no predicate for this one)"
end

window = Window.new(VideoMode.new(720, 360, 32), 'SFML rubyesque: events')
window.frame_rate = 60
frame = 0
last = 'nothing yet -- move the mouse, click, scroll or type'

loop do
  window.poll_events! do |event|
    window.close! if event.closed?
    window.close! if event.key_pressed? && event.code == :escape
    last = describe(event) unless event.closed?
  end

  window.render!(clear_color: [22, 26, 34, 255]) do |target|
    target.draw(ExampleSupport.text(
                  "Last event\n\n#{last}\n\n" \
                  "the predicates are methods, so `event.key_pressed? && event.code == :escape`\n" \
                  'reads like the sentence it is. escape quits.',
                  size: 16, position: [20, 20]
                ))
  end

  frame += 1
  break if ExampleSupport.auto_close?(frame)
end
