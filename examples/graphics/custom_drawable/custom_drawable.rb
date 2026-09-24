# frozen_string_literal: true

# The Drawable contract. Anything that mixes in `SF::Graphics::Drawable` and
# defines `#draw(target, state)` can be handed to `window.draw`, exactly like a
# built-in shape. Here a plain Ruby `Crosshair` composes a CircleShape and a
# VertexArray and forwards them to the `target` it is given, so one `window.draw`
# draws both. A built-in CircleShape is drawn alongside to show the two are
# interchangeable. Move the mouse to steer the crosshair, click to recolor it,
# escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/graphics/custom_drawable/custom_drawable.rb
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

# A custom Drawable: it owns a ring and four tick marks and renders them through
# whatever render target it is drawn into. Window#draw passes a Target, so the
# children are drawn by calling Target#draw -- the same method `window.draw`
# uses. This is composition the C++ Drawable can do too, written in Ruby.
class Crosshair
  include SF::Graphics::Drawable

  TICKS = 4

  def initialize(radius)
    @radius = radius
    @ring = CircleShape.new(radius)
    @ring.origin = [radius, radius]
    @ring.fill_color = [0, 0, 0, 0]
    @ring.outline_thickness = 2
    @ring.outline_color = [120, 230, 160, 255]

    @ticks = VertexArray.new
    @ticks.primitive = :lines
    self.center = [0, 0]
  end

  def center=(point)
    @center = point
    @ring.position = point

    @ticks.clear!
    x = point[0]
    y = point[1]
    TICKS.times do |index|
      angle = index * Math::PI / 2
      inner = @radius - 8
      outer = @radius + 6
      @ticks.append(Vertex.new([x + (Math.cos(angle) * inner), y + (Math.sin(angle) * inner)],
                               @ring.outline_color))
      @ticks.append(Vertex.new([x + (Math.cos(angle) * outer), y + (Math.sin(angle) * outer)],
                               @ring.outline_color))
    end
  end

  def recolor(color)
    @ring.outline_color = color
    self.center = @center
  end

  def draw(target, state)
    target.draw(@ring, state)
    target.draw(@ticks, state)
  end
end

WIDTH = 800
HEIGHT = 600

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML custom Drawable')
window.frame_rate = 60

crosshair = Crosshair.new(46)

# A built-in drawable, to show window.draw treats the two the same.
dot = CircleShape.new(10)
dot.origin = [10, 10]
dot.position = [WIDTH / 2, HEIGHT - 60]
dot.fill_color = [255, 210, 90, 255]

PALETTE = [
  [120, 230, 160, 255], [120, 180, 255, 255],
  [255, 210, 90, 255], [255, 120, 120, 255]
].freeze
color_index = 0

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      window.close! if event.code == :escape
    when 'mouse-moved'
      crosshair.center = [event.mouse_move.x, event.mouse_move.y]
    when 'mouse-button-pressed'
      color_index = (color_index + 1) % PALETTE.length
      crosshair.recolor(PALETTE[color_index])
    end
  end

  window.clear!([18, 20, 30, 255])

  # One call draws the whole custom Drawable (ring + ticks)...
  window.draw(crosshair)
  # ...and one call draws the built-in shape, the identical interface.
  window.draw(dot)

  window.draw(text(
                'Custom Crosshair (includes SF::Graphics::Drawable, defines #draw)',
                size: 17, position: [26, 30]
              ))
  window.draw(text('move the mouse, click to recolor, escape quits',
                   size: 14, position: [26, 56]))
  window.display!

end
