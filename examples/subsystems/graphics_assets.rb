# frozen_string_literal: true

# Sprites, textures and images -- the graphics path the other examples skip.
# It loads the vendored sprite sheet and animates a cell through
# `Sprite#texture_rect`, builds a texture at runtime from `Image.from_pixels`,
# edits an `Image` pixel by pixel, and renders a live scene into a
# `RenderTexture` that is then drawn back as a sprite. S toggles texture
# smoothing, P toggles repeating, escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/graphics_assets.rb
# (headless: SFML_EXAMPLE_FRAMES=120 xvfb-run -a bundle exec ruby -Ilib ...)

require 'sfml'
require_relative '../support'
include SFML

WIDTH = 900
HEIGHT = 560

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML sprites, textures & images')
window.frame_rate = 60

# --- a sprite sheet loaded from disk, animated via texture_rect --------------
BIRD_FRAMES = %i[bird_up bird_mid bird_down bird_mid].freeze
bird = ExampleSupport.sprite(:bird_down, scale: 4)
bird.position = [60, 70]

gem = ExampleSupport.sprite(:gem, scale: 3)
gem.origin = [8, 8]
gem.position = [210, 104]

# --- a procedural texture: an Image block handed to Texture.from_image -------
checker = ExampleSupport.procedural_texture(32, 32) do |x, y|
  ((x / 8) + (y / 8)).even? ? [62, 92, 142, 255] : [92, 132, 192, 255]
end
checker.repeated = true
backdrop = RectangleShape.new([340, 150])
backdrop.position = [520, 40]
backdrop.texture = checker
backdrop.texture_rect = [0, 0, 340, 150]

# --- Image editing -----------------------------------------------------------
gradient = ExampleSupport.image(16, 16) { |x, y| [x * 16, y * 16, 140, 255] }
gradient.set_pixel(0, 0, Color.new(255, 255, 255, 255))
sample = gradient.pixel(0, 0)
buffer = gradient.save_to_memory('png')
gradient.flip_vertically!

# --- RenderTexture: draw offscreen, then draw the result as a sprite ----------
render_texture = RenderTexture.new([240, 150])
render_texture.smooth = true
offscreen_clock = Clock.new

def render_offscreen(render_texture, time)
  render_texture.clear([18, 22, 34, 255])
  render_texture.active = true
  5.times do |index|
    angle = (time * 40) + (index * 72)
    shape = RectangleShape.new([46, 14])
    shape.origin = [23, 7]
    shape.position = [120, 75]
    shape.rotation = angle
    shape.fill_color = Color.new(90 + (index * 30), 140, 230 - (index * 20), 220)
    render_texture.draw(shape)
  end
  render_texture.display
  render_texture.active = false
end

render_offscreen(render_texture, 0.0)
offscreen = Sprite.new(render_texture.texture)
offscreen.position = [520, 250]

event = Event.new
frame = 0
animation = Clock.new
frame_index = 0

loop do
  while window.poll_event!(event)
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      key = event.key[:code]
      window.close! if key == :escape
      if key == :s
        checker.smooth = !checker.smooth?
      elsif key == :p
        checker.repeated = !checker.repeated?
      end
    end
  end

  if animation.elapsed_time.as_seconds > 0.14
    animation.restart!
    frame_index = (frame_index + 1) % BIRD_FRAMES.length
  end
  column, row = ExampleSupport::SPRITES.fetch(BIRD_FRAMES[frame_index])
  bird.texture_rect = [column * ExampleSupport::SHEET_CELL, row * ExampleSupport::SHEET_CELL,
                       ExampleSupport::SHEET_CELL, ExampleSupport::SHEET_CELL]
  gem.rotation += 2

  render_offscreen(render_texture, offscreen_clock.elapsed_time.as_seconds)

  window.clear([14, 16, 24, 255])
  window.draw(backdrop)
  window.draw(bird)
  window.draw(gem)
  window.draw(offscreen)

  window.draw(ExampleSupport.text(
                "Sprite#texture_rect animation over a #{ExampleSupport.sheet.size.x.to_i}x" \
                "#{ExampleSupport.sheet.size.y.to_i} sheet (16x16 cells)\n" \
                'Texture.from_image from Image.from_pixels (32x32 checker) -- ' \
                "smooth? #{checker.smooth?}  repeated? #{checker.repeated?}\n" \
                "Image: size #{gradient.size.x.to_i}x#{gradient.size.y.to_i}  " \
                "pixel(0,0)=#{sample}  save_to_memory(png)=#{buffer ? "#{buffer.size} bytes" : 'nil'}\n" \
                'RenderTexture drawn offscreen, then shown through its #texture',
                size: 15, position: [40, 250]
              ))
  window.draw(ExampleSupport.text('S smoothing, P repeating, escape quits',
                                  size: 14, position: [40, 370]))
  window.display

  frame += 1
  break if ExampleSupport.auto_close?(frame)
end
