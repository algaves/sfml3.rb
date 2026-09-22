# frozen_string_literal: true

# SF::Window::Mouse: the real-time state of the pointer, independent of the event
# queue. `Mouse.position(window)` is relative to the window; `Mouse.position`
# with no argument is relative to the desktop. `Mouse.button_pressed?` and
# `Mouse.pressed?` are synonyms. Click to leave a ripple, C recentres the
# pointer with `Mouse.set_position`, escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/mouse.rb
# (headless: SFML_EXAMPLE_FRAMES=120 xvfb-run -a bundle exec ruby -Ilib ...)

require 'sfml'
require_relative '../support'
include SF::Window
include SF::Graphics
include SF::System
include SF::Audio
include SF::Network

Ripple = Struct.new(:position, :radius, :life)

BUTTONS = %i[left right middle extra1 extra2].freeze

def button_colors
  BUTTONS.each_with_index.to_h { |button, index| [button, 60 + (index * 40)] }
end

def draw_hud(window, _ripples, relative, absolute)
  held = BUTTONS.select { |button| Mouse.button_pressed?(button) }
  window.draw(ExampleSupport.text(
                "window  #{relative.x.to_i}, #{relative.y.to_i}\n" \
                "desktop #{absolute.x.to_i}, #{absolute.y.to_i}\n" \
                "held    #{held.empty? ? '(none)' : held.join(', ')}\n" \
                'click for a ripple, C recentres, escape quits',
                size: 16
              ))

  x = relative.x
  y = relative.y
  window.draw(ExampleSupport.line([[x - 14, y], [x + 14, y]], [235, 235, 245, 200]))
  window.draw(ExampleSupport.line([[x, y - 14], [x, y + 14]], [235, 235, 245, 200]))
end

window = Window.new(VideoMode.new(720, 480, 32), 'SFML mouse')
window.frame_rate = 60
ripples = []
frame = 0
colors = button_colors

loop do
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      key = event.code
      window.close! if key == :escape
      center = [window.size.x / 2, window.size.y / 2]
      Mouse.set_position(center, window) if key == :c
    when 'mouse-button-pressed'
      ripples << Ripple.new(event.mouse_button[:position], 4.0, 255)
    end
  end

  ripples.each do |ripple|
    ripple.radius += 2.5
    ripple.life -= 6
  end
  ripples.reject! { |ripple| ripple.life <= 0 }

  window.clear!([22, 26, 34, 255])

  ripples.each do |ripple|
    ring = CircleShape.new(ripple.radius)
    ring.origin = [ripple.radius, ripple.radius]
    ring.position = [ripple.position.x, ripple.position.y]
    ring.fill_color = [0, 0, 0, 0]
    ring.outline_thickness = 2
    ring.outline_color = [90, 200, 255, ripple.life]
    window.draw(ring)
  end

  BUTTONS.each_with_index do |button, index|
    on = Mouse.button_pressed?(button)
    box = RectangleShape.new([26, 26])
    box.position = [12, 108 + (index * 30)]
    box.fill_color = on ? [colors[button], 230, 120, 255] : [40, 44, 56, 255]
    box.outline_thickness = 1
    box.outline_color = [120, 128, 150, 255]
    window.draw(box)
    window.draw(ExampleSupport.text(button.to_s, size: 15, position: [46, 111 + (index * 30)]))
  end

  draw_hud(window, ripples, Mouse.position(window), Mouse.position)
  window.display!

  frame += 1
  break if ExampleSupport.auto_close?(frame)
end
