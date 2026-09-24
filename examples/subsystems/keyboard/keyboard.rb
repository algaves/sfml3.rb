# frozen_string_literal: true

# SF::Window::Keyboard: real-time key state and the mapping between physical
# scancodes and logical key codes. `key_pressed?` takes a key Symbol (or String
# name, or Integer code); `scancode_pressed?` takes a raw scancode. The HUD
# shows the last key event, `localize` (scancode -> key), `delocalize`
# (key -> scancode) and `description`. T toggles the on-screen virtual keyboard
# (a no-op on desktops without one), escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/keyboard/keyboard.rb
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
      window.draw(text(key.to_s, size: 13, position: [position[0] + 4, position[1] + 12]))
    end
  end
end

window = Window.new(VideoMode.new(520, 380, 32), 'SFML keyboard')
window.frame_rate = 60
last = 'press any key'
virtual_keyboard = false

while window.open?
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
  window.draw(text(
                "#{last}\npressed now: #{held.empty? ? '(none)' : held.join(' ')}\n" \
                "virtual keyboard: #{virtual_keyboard ? 'on' : 'off'} (T toggles), escape quits",
                size: 15, position: [20, 290]
              ))
  window.display!

end
