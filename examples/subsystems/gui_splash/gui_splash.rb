# frozen_string_literal: true

# A splash screen is just a timed animation: scale and fade a logo in, hold it,
# then move on. This recipe drives a logo Sprite with a Clock -- pop-in scale,
# alpha fade, a pulsing "press Enter" prompt -- and then drops into a small
# immediate-mode menu. Enter or a click skips; R replays the splash; escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/gui_splash/gui_splash.rb
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

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML GUI: logo & splash screen')
window.frame_rate = 60

# The logo is an ordinary texture; the window must exist before it is created.
logo_texture = Texture.from_file(File.join(ASSETS, 'logo.png'))
logo = Sprite.new(logo_texture)
logo.origin = [logo_texture.size.x / 2, logo_texture.size.y / 2]
logo.position = [WIDTH / 2, (HEIGHT / 2) - 40]

state = :splash
splash = Clock.new
menu_selected = 0

def alpha_of(color, alpha)
  [color[0], color[1], color[2], alpha]
end

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'mouse-button-pressed'
      state = :menu if state == :splash
    when 'key-pressed'
      case event.code
      when :escape then window.close!
      when :Enter, :Space then state = :menu if state == :splash
      when :r then (state = :splash) && splash.restart!
      when :Up then menu_selected = (menu_selected - 1) % 2 if state == :menu
      when :Down then menu_selected = (menu_selected + 1) % 2 if state == :menu
      end
    end
  end

  window.clear!([16, 18, 28, 255])

  if state == :splash
    t = splash.elapsed_time.as_seconds
    # Ease-out pop: fast at first, settling at full size.
    grow = 0.6 + (0.4 * (1 - Math.exp(-6 * t)))
    fade = [(t / 0.9) * 255, 255].min
    logo.scale = [grow, grow]
    logo.color = alpha_of([255, 255, 255], fade.round)
    window.draw(logo)

    if t > 0.5
      title_fade = [((t - 0.5) / 0.6) * 255, 255].min.round
      window.draw(text('sfml3-rb', size: 34, position: [(WIDTH / 2) - 74, (HEIGHT / 2) + 40],
                                   color: alpha_of([235, 235, 245], title_fade)))
    end

    if t > 1.3
      pulse = (200 + (55 * Math.sin(t * 4))).round
      window.draw(text('press Enter or click to continue', size: 16,
                                                           position: [(WIDTH / 2) - 130, HEIGHT - 90],
                                                           color: alpha_of([150, 160, 190], pulse)))
    end
  else
    window.draw(text('sfml3-rb', size: 40, position: [(WIDTH / 2) - 88, 140]))
    %w[Play Quit].each_with_index do |label, index|
      rect = Rect.new((WIDTH / 2) - 130, 260 + (index * 70), 260, 52)
      box = RectangleShape.new([rect.width, rect.height])
      box.position = [rect.left, rect.top]
      box.fill_color = index == menu_selected ? [90, 170, 230, 255] : [40, 44, 56, 255]
      box.outline_thickness = 1
      box.outline_color = [120, 128, 150, 255]
      window.draw(box)
      window.draw(text(label, size: 20, position: [rect.left + 100, rect.top + 14]))
    end
    window.draw(text('up/down select, R replay splash, escape quits', size: 15, position: [24, HEIGHT - 40]))
  end

  window.display!
end
