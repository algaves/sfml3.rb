# frozen_string_literal: true

# A tiny entity-component-system, and trigger areas built on top of it.
# Entities are integer ids; components are plain hashes keyed by id; systems are
# methods that run over those tables. Use the arrow keys to move the player into
# the glowing trigger zones, which push `[:entered, id]` events onto a queue that
# a score system consumes. Space spawns a drone, D removes the newest, escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/ecs/ecs.rb
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

WIDTH = 860
HEIGHT = 560

# The ECS: component stores keyed by entity id, and one method per system.
class World
  attr_reader :events

  def initialize
    @next_id = 0
    @tags = {}
    @positions = {}
    @velocities = {}
    @shapes = {}
    @zones = {}
    @inside = {}
    @events = []
  end

  def spawn(tag, position:, shape: nil, velocity: nil, zone: nil)
    id = @next_id
    @next_id += 1
    @tags[id] = tag
    @positions[id] = position.dup
    @shapes[id] = shape if shape
    @velocities[id] = velocity if velocity
    @zones[id] = zone if zone
    id
  end

  def despawn(id)
    [@tags, @positions, @velocities, @shapes, @zones, @inside].each { |table| table.delete(id) }
  end

  def ids = @tags.keys
  def tag(id) = @tags[id]
  def position(id) = @positions[id]
  def set_position(id, value) = @positions[id] = value

  # Integrate every entity that has a velocity, bouncing off the window edges.
  def movement_system
    @velocities.each do |id, velocity|
      x, y = @positions[id]
      x += velocity[0]
      y += velocity[1]
      if x < 20 || x > WIDTH - 20
        velocity[0] = -velocity[0]
        x = x.clamp(20, WIDTH - 20)
      end
      if y < 20 || y > HEIGHT - 20
        velocity[1] = -velocity[1]
        y = y.clamp(20, HEIGHT - 20)
      end
      @positions[id] = [x, y]
    end
  end

  # Fire an event only on the frame the player crosses into a zone.
  def trigger_system(player_id)
    px, py = @positions[player_id]
    @zones.each do |id, rect|
      inside = rect.contains?([px, py])
      @events << [:entered, id] if inside && !@inside[id]
      @inside[id] = inside
    end
  end

  def consume_events
    consumed = @events
    @events = []
    consumed
  end

  # Copy each entity's position onto its shape and draw it.
  def render_system(target)
    @shapes.each do |id, shape|
      shape.position = @positions[id]
      target.draw(shape)
    end
  end
end

window = Window.new(VideoMode.new(WIDTH, HEIGHT, 32), 'SFML triggers, events & ECS')
window.frame_rate = 60

def circle(radius, color)
  shape = CircleShape.new(radius)
  shape.origin = [radius, radius]
  shape.fill_color = color
  shape
end

def box(width, height, color)
  shape = RectangleShape.new([width, height])
  shape.fill_color = color
  shape
end

world = World.new

player_shape = circle(16, [90, 170, 230, 255])
player = world.spawn(:player, position: [WIDTH / 2, HEIGHT / 2], shape: player_shape)

# Two patrolling drones: entities whose velocity system drives them.
world.spawn(:drone, position: [180, 140], shape: circle(18, [230, 90, 90, 255]), velocity: [3, 2])
world.spawn(:drone, position: [640, 400], shape: circle(18, [235, 140, 90, 255]), velocity: [-4, 3])

# Two trigger zones: a Rect component plus a translucent shape to draw.
zones = [
  ['north', Rect.new(120, 90, 150, 110)],
  ['east', Rect.new(600, 180, 170, 130)]
]
zones.each do |name, rect|
  shape = box(rect.width, rect.height, [120, 230, 160, 80])
  shape.outline_thickness = 2
  shape.outline_color = [120, 230, 160, 255]
  world.spawn(:"zone:#{name}", position: [rect.left, rect.top], shape: shape, zone: rect)
end

score = 0
log = 'walk into a glowing zone'

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      case event.code
      when :escape then window.close!
      when :Space
        world.spawn(:drone, position: [WIDTH / 2, HEIGHT / 2],
                            shape: circle(14, [180, 110, 235, 255]), velocity: [rand(-5..5), rand(-5..5)])
      when :d
        drones = world.ids.select { |id| world.tag(id) == :drone }
        world.despawn(drones.last) unless drones.empty?
      end
    end
  end

  # Input is one more system: it writes the player's position component.
  speed = 5
  px, py = world.position(player)
  px -= speed if Keyboard.key_pressed?(:Left)
  px += speed if Keyboard.key_pressed?(:Right)
  py -= speed if Keyboard.key_pressed?(:Up)
  py += speed if Keyboard.key_pressed?(:Down)
  world.set_position(player, [px.clamp(0, WIDTH), py.clamp(0, HEIGHT)])

  world.movement_system
  world.trigger_system(player)

  world.consume_events.each do |kind, id|
    next unless kind == :entered

    score += 1
    log = "entered #{world.tag(id)}  (score #{score})"
  end

  window.clear!([14, 16, 26, 255])
  world.render_system(window)
  window.draw(text(
                "entities: #{world.ids.size}   score: #{score}   #{log}\n" \
                'arrows move, space spawn drone, D remove, escape quits',
                size: 15, position: [30, 36]
              ))
  window.display!
end
