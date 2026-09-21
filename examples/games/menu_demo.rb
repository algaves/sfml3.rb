# frozen_string_literal: true

# A GUI built with ExampleSupport::Gui (examples/menu.rb): a draggable control
# panel whose checkboxes, radio group and sliders drive a live preview. Drag the
# panel by its title bar, toggle the grid, pick a quality, move the sliders and
# watch the balls, then Apply or Quit. Escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/games/menu_demo.rb
# (headless: SFML_EXAMPLE_FRAMES=200 xvfb-run -a bundle exec ruby -Ilib ...)

require 'sfml'
require_relative '../support'
require_relative '../menu'
include SFML

WIDTH = 780
HEIGHT = 580
PREVIEW = [390, 40, 350, 500].freeze
QUALITY_SPEED = { 0 => 0.6, 1 => 1.0, 2 => 1.6 }.freeze
PALETTE = [[96, 170, 240], [240, 140, 120], [120, 220, 150],
           [235, 200, 110], [180, 130, 235], [110, 215, 215]].freeze

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML GUI')
window.frame_rate = 60
event = Event.new
input = ExampleSupport::Gui::Input.new(window)

background = ExampleSupport.procedural_texture(4, 220) do |_x, y|
  t = y / 220.0
  [(24 + (t * 26)).to_i, (30 + (t * 34)).to_i, (46 + (t * 52)).to_i, 255]
end

panel = { x: 40, y: 40, dragging: false }
show_grid = true
sound_on = true
quality = 1
volume = 65
ball_count = 16
status = 'Adjust the controls; drag the panel by its title bar.'

def new_ball
  {
    x: PREVIEW[0] + rand(20..(PREVIEW[2] - 20)),
    y: PREVIEW[1] + rand(20..(PREVIEW[3] - 20)),
    vx: [-1.0, 1.0].sample * rand(1.0..3.0),
    vy: [-1.0, 1.0].sample * rand(1.0..3.0),
    color: PALETTE.sample
  }
end

balls = Array.new(ball_count) { new_ball }
frame = 0

loop do
  while window.poll_event!(event)
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      window.close! if event.key[:code] == :escape
    end
  end

  input.update
  ExampleSupport::Gui.begin_frame(window)

  # --- preview, affected by the controls -------------------------------------
  speed = QUALITY_SPEED[quality]
  radius = 5 + (volume / 100.0 * 9)
  target = ball_count.clamp(1, 40)
  balls = balls.first(target)
  balls << new_ball while balls.length < target

  balls.each do |ball|
    ball[:x] += ball[:vx] * speed
    ball[:y] += ball[:vy] * speed
    ball[:vx] = ball[:vx].abs if ball[:x] < PREVIEW[0] + radius
    ball[:vx] = -ball[:vx].abs if ball[:x] > PREVIEW[0] + PREVIEW[2] - radius
    ball[:vy] = ball[:vy].abs if ball[:y] < PREVIEW[1] + radius
    ball[:vy] = -ball[:vy].abs if ball[:y] > PREVIEW[1] + PREVIEW[3] - radius
  end

  window.clear([16, 18, 26, 255])
  backdrop = RectangleShape.new([WIDTH, HEIGHT])
  backdrop.texture = background
  backdrop.texture_rect = [0, 0, WIDTH, HEIGHT]
  window.draw(backdrop)

  ExampleSupport::Gui.rectangle(window, *PREVIEW, [18, 21, 30, 255], [70, 80, 104, 255])
  if show_grid
    1.upto(9) do |index|
      x = PREVIEW[0] + (PREVIEW[2] * index / 10)
      y = PREVIEW[1] + (PREVIEW[3] * index / 10)
      window.draw(ExampleSupport.line([[x, PREVIEW[1]], [x, PREVIEW[1] + PREVIEW[3]]], [40, 48, 66, 255]))
      window.draw(ExampleSupport.line([[PREVIEW[0], y], [PREVIEW[0] + PREVIEW[2], y]], [40, 48, 66, 255]))
    end
  end

  balls.each do |ball|
    dot = CircleShape.new(radius)
    dot.origin = [radius, radius]
    dot.position = [ball[:x], ball[:y]]
    dot.fill_color = ball[:color]
    window.draw(dot)
  end

  # --- control panel ---------------------------------------------------------
  ExampleSupport::Gui.draggable_panel(window, input, panel, 'Control Panel', 320, 500)
  px = panel[:x]
  py = panel[:y]

  ExampleSupport::Gui.label(window, 'Preview', px + 16, py + 40, size: 14, color: ExampleSupport::Gui::MUTED)
  show_grid = ExampleSupport::Gui.checkbox(window, input, 'Show grid', px + 16, py + 62, 200, 26, show_grid)
  sound_on = ExampleSupport::Gui.checkbox(window, input, 'Sound', px + 16, py + 94, 200, 26, sound_on)

  ExampleSupport::Gui.label(window, "Quality  (#{sound_on ? 'sound on' : 'muted'})",
                            px + 16, py + 130, size: 14, color: ExampleSupport::Gui::MUTED)
  quality = ExampleSupport::Gui.radio(window, input, %w[Low Medium High],
                                      px + 20, py + 154, 200, 78, quality)

  volume = ExampleSupport::Gui.slider(window, input, 'Volume', px + 20, py + 274, 260, 18, volume, 0, 100)
  ball_count = ExampleSupport::Gui.slider(window, input, 'Balls', px + 20, py + 334, 260, 18, ball_count, 1, 40)
  ExampleSupport::Gui.gauge(window, 'Level', px + 20, py + 384, 260, 14, volume / 100.0)

  if ExampleSupport::Gui.button(window, input, 'Apply', px + 20, py + 420, 120, 34)
    status = "Applied: quality #{%w[Low Medium High][quality]}, volume #{volume.to_i}, " \
             "#{ball_count.to_i} balls, grid #{show_grid ? 'on' : 'off'}"
  end
  window.close! if ExampleSupport::Gui.button(window, input, 'Quit', px + 160, py + 420, 120, 34)

  window.draw(ExampleSupport.text(status, size: 15, position: [40, HEIGHT - 30], color: [200, 208, 230, 255]))

  window.display
  frame += 1
  break if ExampleSupport.auto_close?(frame)
end
