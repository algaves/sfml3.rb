# frozen_string_literal: true

# SF::Audio::Listener: the point in the scene from which all sounds are heard. The
# listener has a position, a direction, an up-vector and a global volume; a
# spatialized Sound carries its own position, cone and attenuation, and its
# loudness is computed from the distance between the two. Here the source is
# pinned at the right of the world and the listener moves with the arrow keys
# (PageUp/PageDown change its height). Space toggles playback, +/- the global
# volume, escape quits.
#
# Run from the repository root with a display and an audio device:
#   bundle exec ruby -Ilib examples/subsystems/listener/listener.rb
# (headless: prefix `xvfb-run -a`).

require 'sfml'
include SF::Window
include SF::Graphics
include SF::System
include SF::Audio

# --- self-contained helpers -------------------------------------------------
ASSETS = File.expand_path('assets', __dir__)

FONT = Font.from_file(File.join(ASSETS, 'LiberationSans-Regular.ttf'))
def text(string, size: 18, position: [12, 8], color: [235, 235, 245, 255])
  label = Text.new(FONT, string, size)
  label.fill_color = color
  label.position = position
  label
end

def sound(name, volume: 60)
  buffer = SoundBuffer.from_file(File.join(ASSETS, "#{name}.wav"))
  Sound.new(buffer).tap { |sound| sound.volume = volume }
end

def line(points, color)
  array = VertexArray.new
  points.each { |pair| array.append(Vertex.new(pair, color)) }
  array.primitive = :lines
  array
end

SOURCE = Vector3.new(520, 240, 0)

def distance(a, b)
  Math.sqrt(((a.x - b.x)**2) + ((a.y - b.y)**2) + ((a.z - b.z)**2))
end

def draw_scene(window, listener, sound_on)
  window.draw(line([[listener.x, listener.y], [SOURCE.x, SOURCE.y]],
                   [90, 100, 125, 255]))

  source = CircleShape.new(18)
  source.origin = [18, 18]
  source.position = [SOURCE.x, SOURCE.y]
  source.fill_color = sound_on ? [235, 200, 90, 255] : [130, 120, 80, 255]
  window.draw(source)
  window.draw(text('sound source', size: 14, position: [SOURCE.x - 42, SOURCE.y + 24]))

  arrow = ConvexShape.new(3)
  arrow.set_point(0, [16, 0])
  arrow.set_point(1, [-10, 10])
  arrow.set_point(2, [-10, -10])
  arrow.origin = [16, 0]
  arrow.position = [listener.x, listener.y]
  arrow.fill_color = [90, 200, 255, 255]
  window.draw(arrow)

  zoom = CircleShape.new(6)
  zoom.origin = [6, 6]
  zoom.position = [listener.x, listener.y]
  zoom.fill_color = [235, 235, 245, 255]
  window.draw(zoom)
  window.draw(text('listener', size: 14, position: [listener.x - 26, listener.y + 18]))
end

window = Window.new(VideoMode.new(640, 480, 32), 'SFML listener')
window.frame_rate = 60

sound = sound('beep')
sound.looping = true
sound.spatialization_enabled = true
sound.position = [SOURCE.x, SOURCE.y, SOURCE.z]
sound.cone = SoundSourceCone.new(90, 240, 0.4)
sound.attenuation = 1.0
sound.min_distance = 60
sound.max_distance = 600
sound.play!

Listener.position = [150, 240, 0]
Listener.direction = [0, 0, -1]
Listener.up_vector = [0, 1, 0]
Listener.global_volume = 80

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      key = event.code
      window.close! if key == :escape
      sound.play! if key == :Space && sound.stopped?
      sound.pause! if key == :Space && sound.playing?
      sound.playing_offset = SF::System::Time.zero if key == :r
    end
  end

  # Held keys move the listener each frame.
  position = Listener.position
  x = position.x
  y = position.y
  z = position.z
  x -= 8 if Keyboard.key_pressed?(:Left)
  x += 8 if Keyboard.key_pressed?(:Right)
  y -= 8 if Keyboard.key_pressed?(:Up)
  y += 8 if Keyboard.key_pressed?(:Down)
  z -= 8 if Keyboard.key_pressed?(:PageDown)
  z += 8 if Keyboard.key_pressed?(:PageUp)
  volume = Listener.global_volume
  volume += 4 if Keyboard.key_pressed?(:Add)
  volume -= 4 if Keyboard.key_pressed?(:Subtract)
  Listener.global_volume = volume.clamp(0, 100)
  Listener.position = [x.clamp(0, 640), y.clamp(0, 480), z]

  listener = Listener.position
  window.clear!([22, 26, 34, 255])
  draw_scene(window, listener, !sound.stopped?)
  window.draw(text(
                "listener  #{listener.x.to_i}, #{listener.y.to_i}, #{listener.z.to_i}\n" \
                "distance  #{distance(listener, SOURCE).round} units\n" \
                "volume    #{Listener.global_volume.round}%\n" \
                "sound     #{sound.status} (Space toggles, R rewinds)\n" \
                'arrows move, PageUp/Down change height, escape quits',
                size: 15
              ))
  window.display!

end
