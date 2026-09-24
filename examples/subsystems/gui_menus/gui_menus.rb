# frozen_string_literal: true

# An immediate-mode GUI: the menu is described from scratch every frame rather
# than kept as widget objects. Buttons know nothing; the loop measures the mouse
# against each rectangle, draws the hovered/selected one highlighted, and runs an
# action when it is clicked or chosen with the keyboard. There are three screens:
# a menu, an options screen with a slider, and a playing screen with a HUD.
# Up/Down or the mouse select, Enter or a click activates, escape backs out of a
# screen (and quits from the menu).
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/gui_menus/gui_menus.rb
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

WIDTH = 900
HEIGHT = 620

Button = Struct.new(:label, :action)

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML GUI: menus & HUD')
window.frame_rate = 60

buttons = [
  Button.new('New Game', :new_game),
  Button.new('Options', :options),
  Button.new('Quit', :quit)
]

def button_bounds(index)
  Rect.new((WIDTH / 2) - 130, 220 + (index * 70), 260, 52)
end

state = :menu
selected = 0
volume = 70
play_clock = Clock.new

while window.open?
  mouse = Mouse.position(window)
  hover = buttons.each_index.find { |index| button_bounds(index).contains?(mouse) }

  activate = lambda do |action|
    case action
    when :new_game
      state = :playing
      play_clock.restart!
    when :options
      state = :options
    when :quit
      window.close!
    end
  end

  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'mouse-button-pressed'
      activate.call(buttons[hover].action) if event.mouse_button[:button] == :left && hover
    when 'key-pressed'
      case event.code
      when :escape
        state == :menu ? window.close! : state = :menu
      when :Up then selected = (selected - 1) % buttons.length
      when :Down then selected = (selected + 1) % buttons.length
      when :Enter then activate.call(buttons[selected].action)
      when :Left then volume = [volume - 5, 0].max if state == :options
      when :Right then volume = [volume + 5, 100].min if state == :options
      end
    end
  end

  window.clear!([16, 18, 28, 255])

  if state == :menu
    window.draw(text('sfml3-rb', size: 40, position: [(WIDTH / 2) - 88, 120], color: [235, 235, 245, 255]))

    buttons.each_with_index do |button, index|
      rect = button_bounds(index)
      highlighted = index == hover || index == selected
      box = RectangleShape.new([rect.width, rect.height])
      box.position = [rect.left, rect.top]
      box.fill_color = highlighted ? [90, 170, 230, 255] : [40, 44, 56, 255]
      box.outline_thickness = 1
      box.outline_color = [120, 128, 150, 255]
      window.draw(box)
      window.draw(text(button.label, size: 20, position: [rect.left + 24, rect.top + 14]))
    end
  elsif state == :options
    window.draw(text('Options', size: 28, position: [60, 60]))
    window.draw(text("Master volume: #{volume}", size: 18, position: [60, 140]))
    track = RectangleShape.new([300, 10])
    track.position = [60, 180]
    track.fill_color = [40, 44, 56, 255]
    window.draw(track)
    knob = RectangleShape.new([10, 24])
    knob.position = [60 + (volume * 3) - 5, 173]
    knob.fill_color = [90, 170, 230, 255]
    window.draw(knob)
    window.draw(text('left/right adjust, escape back to the menu', size: 15, position: [60, 240]))
  else
    window.draw(text('Playing', size: 34, position: [(WIDTH / 2) - 60, 60]))
    window.draw(text("elapsed: #{play_clock.elapsed_time.as_seconds.round(1)}s", size: 20, position: [60, 140]))
    window.draw(text('This is the in-game HUD. escape returns to the menu.', size: 16, position: [60, 180]))
  end

  window.display!
end
