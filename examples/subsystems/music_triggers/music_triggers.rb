# frozen_string_literal: true

# Music and sound effects driven by gameplay events. A looping `Music` plays
# underneath; a pickup sound fires when the player reaches the gem, a bump sound
# when a patroller hits, and a chime fires on a timer (a "moment" trigger), not
# on input. M pauses/resumes the music, -/= change its volume, escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/music_triggers/music_triggers.rb
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

WIDTH = 820
HEIGHT = 560

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML music & event-triggered sound')
window.frame_rate = 60

# A streamed track that loops forever, plus two short effects.
music = Music.from_file(File.join(ASSETS, 'beep.wav'))
music.looping = true
music.volume = 35
music.play!

pickup = Sound.new(SoundBuffer.from_file(File.join(ASSETS, 'beep.wav')))
pickup.volume = 80
bump = Sound.new(SoundBuffer.from_file(File.join(ASSETS, 'bounce.wav')))
bump.volume = 90

player = CircleShape.new(16)
player.origin = [16, 16]
player.fill_color = [90, 170, 230, 255]
player.position = [WIDTH / 2, HEIGHT / 2]

gem = CircleShape.new(12)
gem.origin = [12, 12]
gem.fill_color = [255, 210, 90, 255]
gem.position = [120, 120]

Patroller = Struct.new(:shape, :vx, :vy)

patrollers = [[220, 160, 4, 0], [600, 400, -5, 3]].map do |x, y, vx, vy|
  shape = RectangleShape.new([34, 34])
  shape.position = [x, y]
  shape.fill_color = [230, 90, 90, 255]
  Patroller.new(shape, vx, vy)
end

score = 0
flash = 0.0
message = 'collect the gem'
chime = Clock.new
music_on = true

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      case event.code
      when :escape then window.close!
      when :m
        music_on = !music_on
        music_on ? music.play! : music.pause!
      when :Equal, :Add then music.volume = [music.volume + 5, 100].min
      when :Hyphen, :Subtract then music.volume = [music.volume - 5, 0].max
      end
    end
  end

  speed = 4
  player.move([-speed, 0]) if Keyboard.key_pressed?(:Left)
  player.move([speed, 0]) if Keyboard.key_pressed?(:Right)
  player.move([0, -speed]) if Keyboard.key_pressed?(:Up)
  player.move([0, speed]) if Keyboard.key_pressed?(:Down)

  # Gameplay event: the player reached the gem.
  if (player.position.x - gem.position.x).abs < 24 && (player.position.y - gem.position.y).abs < 24
    pickup.play!
    score += 1
    flash = 0.2
    message = "pickup ##{score}"
    gem.position = [rand(60..(WIDTH - 60)), rand(80..(HEIGHT - 60))]
  end

  patrollers.each do |patroller|
    shape = patroller.shape
    position = shape.position
    patroller.vx = -patroller.vx if position.x < 20 || position.x > WIDTH - 54
    patroller.vy = -patroller.vy if position.y < 20 || position.y > HEIGHT - 54
    shape.position = [position.x + patroller.vx, position.y + patroller.vy]

    # Gameplay event: a patroller hit the player.
    next unless (player.position.x - shape.position.x).abs < 42 && (player.position.y - shape.position.y).abs < 42

    bump.play!
    player.move([0, -6])
    message = 'bumped!'
    flash = 0.35
  end

  # Moment event: a timed chime every five seconds, independent of input.
  if chime.elapsed_time.as_seconds > 5.0
    chime.restart!
    pickup.play!
    message = 'chime!'
    flash = 0.3
  end

  flash = [flash - 0.02, 0.0].max

  window.clear!([16, 18, 28, 255])
  if flash.positive?
    wash = RectangleShape.new([WIDTH, HEIGHT])
    wash.fill_color = [255, 255, 255, (flash * 255).to_i]
    window.draw(wash)
  end
  window.draw(gem)
  patrollers.each { |patroller| window.draw(patroller.shape) }
  window.draw(player)
  window.draw(text(
                "score: #{score}   music: #{music_on ? "on, vol #{music.volume.to_i}" : 'paused'}   " \
                "status: #{music.status}   #{message}\n" \
                'arrows move, M music, -/= volume, escape quits',
                size: 15, position: [30, 36]
              ))
  window.display!
end
