# frozen_string_literal: true

# Image -- the CPU-side pixel buffer -- and its trip to the GPU. Builds an Image
# from raw pixels, edits single pixels, blits one Image into another with
# copy_image, keys out a colour with create_mask_from_color, flips it, saves it
# to a temp PNG and to memory, reloads it, and uploads it with
# Texture.from_image (then reads it back with Texture#copy_to_image). F flips,
# V flips vertically, M applies the colour-key mask, S saves+reloads, R resets,
# escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/graphics/image_pixels/image_pixels.rb
# (headless: prefix `xvfb-run -a`).

require 'sfml'
require 'tmpdir'
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

# Builds a CPU Image from a block returning [r, g, b] or [r, g, b, a].
def image(width, height)
  pixels = String.new(encoding: Encoding::BINARY)
  height.times do |y|
    width.times do |x|
      color = yield(x, y)
      color += [255] if color.size == 3
      pixels << color.pack('C4')
    end
  end
  Image.from_pixels([width, height], pixels)
end

KEY_COLOR = Color.new(255, 0, 255, 255)

# A gradient with a magenta block in the top-left corner -- the colour the mask
# will key out.
def gradient
  image(64, 64) do |x, y|
    if x < 10 && y < 10
      [255, 0, 255, 255]
    else
      [(x * 4), (y * 4), 140, 255]
    end
  end
end

WIDTH = 800
HEIGHT = 600

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML Image & Texture pixels')
window.frame_rate = 60

picture = gradient
picture.set_pixel(0, 0, Color.new(255, 255, 255, 255)) # one white pixel at the origin

# Blit a 16x16 region of the gradient into a flat-coloured Image.
canvas = Image.from_color([64, 64], Color.new(28, 34, 52, 255))
canvas.copy_image(picture, [16, 16], Rect.new(4, 4, 16, 16), false)

texture = Texture.from_image(picture)
texture.smooth = false # keep the pixels crisp when scaled up
canvas_texture = Texture.from_image(canvas)
canvas_texture.smooth = false
dirty = false

saved_path = File.join(Dir.tmpdir, 'sfml_image_pixels.png')
info = { saved: '-', reloaded: '-', memory: '-', roundtrip: '-' }

reload_texture = lambda do
  texture = Texture.from_image(picture)
  texture.smooth = false
  dirty = false
end

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      case event.code
      when :escape
        window.close!
      when :f
        picture.flip_horizontally!
        dirty = true
      when :v
        picture.flip_vertically!
        dirty = true
      when :m
        picture.create_mask_from_color(KEY_COLOR, 0)
        dirty = true
      when :s
        ok = picture.save_to_file(saved_path)
        reloaded = Image.from_file(saved_path)
        memory = picture.save_to_memory('png')
        roundtrip = texture.copy_to_image
        info = {
          saved: (ok ? 'yes' : 'no'),
          reloaded: "#{reloaded.size.x.to_i}x#{reloaded.size.y.to_i}",
          memory: (memory ? "#{memory.size} bytes" : 'nil'),
          roundtrip: "#{roundtrip.size.x.to_i}x#{roundtrip.size.y.to_i}"
        }
      when :r
        picture = gradient
        picture.set_pixel(0, 0, Color.new(255, 255, 255, 255))
        dirty = true
      end
    end
  end

  reload_texture.call if dirty

  sample = picture.pixel(0, 0)

  window.clear!([16, 18, 26, 255])

  # The CPU Image, uploaded to a Texture and stretched 5x.
  scaled = RectangleShape.new([320, 320])
  scaled.position = [50, 150]
  scaled.texture = texture
  scaled.texture_rect = [0, 0, 64, 64]
  scaled.scale = [5, 5]
  window.draw(scaled)

  # The blitted canvas, uploaded separately.
  canvas_sprite = RectangleShape.new([240, 240])
  canvas_sprite.position = [430, 150]
  canvas_sprite.texture = canvas_texture
  canvas_sprite.texture_rect = [0, 0, 64, 64]
  canvas_sprite.scale = [3.75, 3.75]
  window.draw(canvas_sprite)

  window.draw(text('Image (CPU pixels) -> Texture (GPU)', size: 18, position: [50, 110]))
  window.draw(text('Texture.from_image + Texture#copy_to_image', size: 15, position: [430, 110]))
  window.draw(text(
                "size #{picture.size.x.to_i}x#{picture.size.y.to_i}   pixel(0,0) = " \
                "rgba(#{sample.to_a.join(',')})\n" \
                "save_to_file #{info[:saved]} -> reload #{info[:reloaded]}, " \
                "save_to_memory -> #{info[:memory]}, round-trip #{info[:roundtrip]}\n" \
                'F flip, V flip-v, M colour-key mask, S save+reload, R reset, escape quits',
                size: 15, position: [50, 500]
              ))
  window.display!

end
