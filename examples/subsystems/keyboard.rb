# frozen_string_literal: true

# SF::Window::Keyboard: real-time key state and the mapping between physical
# scancodes and logical key codes. `key_pressed?` takes a key Symbol (or String
# name, or Integer code); `scancode_pressed?` takes a raw scancode. The HUD
# shows the last key event, `localize` (scancode -> key), `delocalize`
# (key -> scancode) and `description`. T toggles the on-screen virtual keyboard
# (a no-op on desktops without one), escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/keyboard.rb
# (headless: SFML_EXAMPLE_FRAMES=120 xvfb-run -a bundle exec ruby -Ilib ...)

require 'sfml'
require_relative '../support'
include SF::Window
include SF::Graphics
include SF::System
include SF::Audio
include SF::Network

ROWS = [
  %i[num1 num2 num3 num4 num5 num6 num7 num8 num9 num0],
  %i[q w e r t y u i o p],
  %i[a s d f g h j k l],
  %i[z x c v b n m],
  %i[Space]
].freeze

KEY_SIZE = 40
KEY_GAP = 6

def key_box(_key, position)
  box = RectangleShape.new([KEY_SIZE, KEY_SIZE])
  box.position = position
  box.outline_thickness = 1
  box.outline_color = [120, 128, 150, 255]
  box
end

def draw_keyboard(window)
  ROWS.each_with_index do |row, row_index|
    offset = (10 - row.size) * (KEY_SIZE + KEY_GAP) / 2
    row.each_with_index do |key, column|
      position = [20 + offset + (column * (KEY_SIZE + KEY_GAP)), 70 + (row_index * (KEY_SIZE + KEY_GAP))]
      box = key_box(key, position)
      box.fill_color = Keyboard.key_pressed?(key) ? [90, 180, 255, 255] : [40, 44, 56, 255]
      window.draw(box)
      window.draw(ExampleSupport.text(key.to_s, size: 13, position: [position[0] + 4, position[1] + 12]))
    end
  end
end

window = Window.new(VideoMode.new(520, 380, 32), 'SFML keyboard')
window.frame_rate = 60
frame = 0
last = 'press any key'
virtual_keyboard = false

loop do
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      key = event.key
      window.close! if key[:code] == :escape
      if key[:code] == :t
        virtual_keyboard = !virtual_keyboard
        Keyboard.virtual_keyboard_visible = virtual_keyboard
      end
      local = Keyboard.localize(key[:scancode])
      last = "#{key[:code]}  scancode #{key[:scancode]}  " \
             "shift=#{key[:shift]} ctrl=#{key[:control]} alt=#{key[:alt]}\n" \
             "description \"#{Keyboard.description(key[:scancode])}\"  " \
             "localize=#{local}  delocalize=#{Keyboard.delocalize(local)}"
    end
  end

  held = ROWS.flatten.select { |key| Keyboard.key_pressed?(key) }

  window.clear!([22, 26, 34, 255])
  draw_keyboard(window)
  window.draw(ExampleSupport.text(
                "#{last}\npressed now: #{held.empty? ? '(none)' : held.join(' ')}\n" \
                "virtual keyboard: #{virtual_keyboard ? 'on' : 'off'} (T toggles), escape quits",
                size: 15, position: [20, 290]
              ))
  window.display!

  frame += 1
  break if ExampleSupport.auto_close?(frame)
end
