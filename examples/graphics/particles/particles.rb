# frozen_string_literal: true

# A particle system is a pool of tiny, short-lived pieces of geometry reused
# every frame. Each particle carries a position, a velocity, a lifetime and a
# colour; every frame the loop spawns new ones at a rate, integrates velocity
# and gravity, fades them out, and drops the dead. All of them are drawn from
# one VertexArray of triangles, so thousands cost a single draw call.
#
# Click to burst, space toggles gravity, +/- change the spawn rate, escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/graphics/particles/particles.rb
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

# One particle is plain data, not a class hierarchy: recycling these is the
# whole trick to keeping a particle system cheap.
PARTICLE = Struct.new(:x, :y, :vx, :vy, :life, :max_life, :diameter)
MAX_PARTICLES = 6000

WIDTH = 900
HEIGHT = 560

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML particle system')
window.frame_rate = 60

clock = Clock.new
particles = []
spawn_rate = 600.0
spawn_accumulator = 0.0
gravity = 240.0
emitter = [WIDTH / 2, (HEIGHT / 2) + 60]

def spawn(list, x, y, count, speed)
  count.times do
    angle = rand * 2 * Math::PI
    magnitude = speed * (0.2 + (rand * 0.8))
    life = 0.5 + (rand * 1.1)
    list << PARTICLE.new(x, y, Math.cos(angle) * magnitude, Math.sin(angle) * magnitude,
                         life, life, rand(2.0..5.0))
  end
end

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'mouse-button-pressed'
      pointer = Mouse.position(window)
      spawn(particles, pointer.x, pointer.y, 250, 320.0)
    when 'key-pressed'
      case event.code
      when :escape then window.close!
      when :Space then gravity = gravity.zero? ? 240.0 : 0.0
      when :Equal then spawn_rate = [spawn_rate + 100, 2000.0].min
      when :Hyphen then spawn_rate = [spawn_rate - 100, 0.0].max
      when :c then particles.clear
      end
    end
  end

  dt = clock.restart!.as_seconds
  dt = 0.05 if dt > 0.05 # clamp after a stall so nothing teleports

  # Spawn at a steady rate, independent of the frame rate.
  spawn_accumulator += spawn_rate * dt
  while spawn_accumulator >= 1.0
    spawn_accumulator -= 1.0
    spawn(particles, emitter[0], emitter[1], 1, 180.0)
  end

  # Integrate and retire.
  particles.reject! do |p|
    p.vy += gravity * dt
    p.x += p.vx * dt
    p.y += p.vy * dt
    p.life -= dt
    p.life <= 0
  end
  particles.shift(particles.length - MAX_PARTICLES) if particles.length > MAX_PARTICLES

  # Build the geometry: one quad (two triangles) per particle, coloured by age.
  mesh = VertexArray.new
  mesh.primitive = :triangles
  particles.each do |p|
    fade = p.life / p.max_life
    red = 255
    green = (90 + (150 * fade)).round
    blue = (40 * fade).round
    alpha = (255 * fade).round
    color = [red, green, blue, alpha]
    half = p.diameter / 2
    left = p.x - half
    right = p.x + half
    top = p.y - half
    bottom = p.y + half
    mesh.append(Vertex.new([left, top], color))
    mesh.append(Vertex.new([right, top], color))
    mesh.append(Vertex.new([right, bottom], color))
    mesh.append(Vertex.new([left, top], color))
    mesh.append(Vertex.new([right, bottom], color))
    mesh.append(Vertex.new([left, bottom], color))
  end

  window.clear!([12, 14, 22, 255])
  window.draw(mesh)

  window.draw(text(
                "particles #{particles.length}/#{MAX_PARTICLES}   " \
                "spawn #{spawn_rate.to_i}/s   gravity #{gravity.zero? ? 'off' : 'on'}\n" \
                'click bursts, space gravity, +/- rate, C clears, escape quits',
                size: 15
              ))
  window.display!
end
