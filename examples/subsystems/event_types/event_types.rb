# frozen_string_literal: true

# Every event SFML reports has a type string and, for each kind that carries
# data, a payload hash (`event.key`, `event.mouse_button`, `event.touch`, ...).
# This recipe logs one line per event with the payload, over a spinning marker
# that proves frames keep running between events. Move, click, scroll, type,
# resize, plug in a gamepad or touch the screen and watch the log.
#
# Escape quits. For inspect-vs-wait (poll_event! / wait_event!) see the
# `examples/subsystems/event_wait/` script.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/event_types/event_types.rb
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

# One sentence per event kind, using the payload hashes (the same ones every
# event always had) and the `event.code` shortcut for `event.key[:code]`.
def describe(event)
  return 'closed' if event.closed?
  return "resized to #{event.size.x.to_i}x#{event.size.y.to_i}" if event.resized?
  return 'gained focus' if event.gained_focus?
  return 'lost focus' if event.lost_focus?
  return "text entered: #{event.text.inspect}" if event.text_entered?
  return "key #{event.code} pressed" if event.key_pressed?
  return "key #{event.code} released" if event.key_released?
  return "wheel scrolled, delta #{event.mouse_wheel_scroll[:delta]}" if event.mouse_wheel_scrolled?
  if event.mouse_button_pressed?
    return "mouse button #{event.mouse_button[:button]} pressed at #{button_position(event)}"
  end
  return "mouse button #{event.mouse_button[:button]} released" if event.mouse_button_released?
  return "mouse moved to #{event.mouse_move.x.to_i},#{event.mouse_move.y.to_i}" if event.mouse_moved?
  return "mouse moved (raw) #{event.mouse_move_raw.x.to_i},#{event.mouse_move_raw.y.to_i}" if event.mouse_moved_raw?
  return 'mouse entered the window' if event.mouse_entered?
  return 'mouse left the window' if event.mouse_left?
  return "joystick #{event.joystick_connect[:joystick_id]} connected" if event.joystick_connected?
  return "joystick #{event.joystick_connect[:joystick_id]} disconnected" if event.joystick_disconnected?
  if event.joystick_moved?
    return "joystick #{event.joystick_move[:joystick_id]} axis #{event.joystick_move[:axis]} moved"
  end
  if event.joystick_button_pressed?
    return "joystick #{event.joystick_button[:joystick_id]} button #{event.joystick_button[:button]} pressed"
  end
  return 'joystick button released' if event.joystick_button_released?
  return "touch began, finger #{event.touch[:finger]} at #{touch_position(event)}" if event.touch_began?
  return "touch moved, finger #{event.touch[:finger]}" if event.touch_moved?
  return "touch ended, finger #{event.touch[:finger]}" if event.touch_ended?
  return "sensor #{event.sensor[:type]} changed" if event.sensor_changed?

  "#{event.type} (no payload)"
end

def button_position(event)
  x = event.mouse_button[:x].to_i
  y = event.mouse_button[:y].to_i
  "#{x},#{y}"
end

def touch_position(event)
  x = event.touch[:x].to_i
  y = event.touch[:y].to_i
  "#{x},#{y}"
end

WIDTH = 900
HEIGHT = 560

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML events: every event type')
window.frame_rate = 60

spinner = Clock.new
log = ['waiting for events -- move, click, scroll, type, resize or touch']

while window.open?
  window.poll_events! do |event|
    window.close! if event.closed? || (event.key_pressed? && event.code == :escape)
    next if event.closed?

    log.unshift(describe(event))
    log.pop while log.length > 14
  end

  # A marker that rotates continuously, so you can tell frames keep running.
  angle = spinner.elapsed_time.as_seconds * 3
  hand = VertexArray.new
  hand.primitive = :lines
  hand.append(Vertex.new([WIDTH - 70, 70], [120, 128, 150, 255]))
  hand.append(Vertex.new([WIDTH - 70 + (Math.cos(angle) * 40), 70 + (Math.sin(angle) * 40)],
                         [90, 170, 230, 255]))

  window.clear!([16, 18, 28, 255])
  window.draw(hand)

  lines = log.each_with_index.map do |entry, index|
    prefix = index.zero? ? '> ' : '  '
    "#{prefix}#{entry}"
  end
  window.draw(text("event log (newest first)\n\n#{lines.join("\n")}", size: 15, position: [30, 30]))
  window.draw(text('escape quits', size: 15, position: [30, HEIGHT - 30], color: [150, 160, 190, 255]))
  window.display!
end
