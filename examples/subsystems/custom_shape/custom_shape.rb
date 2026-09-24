# frozen_string_literal: true

# The `Shape` base class builds its polygon from two questions you answer:
# `#point_count` (how many corners) and `#point(index)` (where each one is).
# Call `update!` to refresh after changing the geometry. Here a `Star` subclass
# is re-tuned live: +/- change its radius, [ / ] change its point count, space
# swaps fill and outline. Escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/custom_shape/custom_shape.rb
# (headless: prefix `xvfb-run -a`).

require 'sfml'
include SF::Window
include SF::Graphics
include SF::System

# A Shape subclass only has to say how many points it has and where they sit.
# Alternating the radius between points is what makes a star.
class Star < Shape
  attr_accessor :radius, :spikes

  def initialize(radius, spikes)
    super()
    @radius = radius
    @spikes = spikes
    update!
  end

  def point_count
    @spikes * 2
  end

  def point(index)
    angle = (index * Math::PI) / @spikes
    length = index.even? ? @radius : @radius * 0.45
    [Math.cos(angle) * length, Math.sin(angle) * length]
  end

  def regrow!
    update!
  end
end

window = Window.new(VideoMode.new(800, 600, 32), 'SFML custom shape: subclassing Shape')
window.frame_rate = 60

star = Star.new(90, 5)
star.position = [400, 280]
star.origin = [0, 0]
star.fill_color = [250, 220, 70, 255]

outline = Star.new(160, 7)
outline.position = [400, 280]
outline.fill_color = [0, 0, 0, 0]
outline.outline_thickness = 2
outline.outline_color = [90, 170, 230, 255]

filled = true

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      case event.code
      when :escape
        window.close!
      when :Space
        filled = !filled
        star.fill_color = filled ? [250, 220, 70, 255] : [0, 0, 0, 0]
        star.outline_thickness = filled ? 0 : 3
        star.outline_color = [250, 220, 70, 255]
      when :Equal
        star.radius += 10
        star.regrow!
      when :Hyphen
        star.radius = [star.radius - 10, 10].max
        star.regrow!
      when :RBracket
        star.spikes = [star.spikes + 1, 12].min
        star.regrow!
      when :LBracket
        star.spikes = [star.spikes - 1, 3].max
        star.regrow!
      end
    end
  end

  star.rotation += 1
  outline.rotation -= 0.5

  window.clear!([10, 10, 20, 255])
  window.draw(star)
  window.draw(outline)
  window.display!
end
