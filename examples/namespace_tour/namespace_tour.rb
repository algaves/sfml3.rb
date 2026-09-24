# frozen_string_literal: true

# A guided tour of the `SF` namespace, printed to the console. It walks the root
# module and each of its five subsystem modules, lists the classes each one
# holds, shows the `include SF::<Subsystem>` shorthand the examples use, and
# confirms that `SFML` is a deprecated alias of `SF`. Console-only: no display,
# no audio device, no assets, and it exits on its own.
#
# Run from the repository root:
#   bundle exec ruby -Ilib examples/namespace_tour/namespace_tour.rb

require 'sfml'

SUBSYSTEMS = %i[System Graphics Window Audio Network].freeze

NOTES = {
  System: 'values and timing: Vector2/Vector3, Time, Clock, sleep, InputStream, Buffer',
  Graphics: 'what you draw and draw onto: shapes, sprites, textures, text, views, shaders',
  Window: 'the OS window, its events, and the input devices (Keyboard, Mouse, Joystick...)',
  Audio: 'playback and capture: Sound, Music, SoundBuffer, recorders, Listener',
  Network: 'sockets and protocols: TcpSocket/TcpListener, UdpSocket, Packet, Http, Ftp'
}.freeze

# The constants defined directly on +namespace+, as sorted [name, kind] pairs.
# `false` skips anything inherited, so each subsystem lists only its own parts.
def own_constants(namespace)
  namespace.constants(false).sort.filter_map do |name|
    value = namespace.const_get(name, false)
    next unless value.is_a?(Module)

    [name, value.is_a?(Class) ? 'class' : 'module']
  end
end

def print_constants(namespace, title, note)
  entries = own_constants(namespace)
  puts "#{title}  (#{entries.length} entries)"
  puts "  #{note}"
  entries.each { |name, kind| puts format('    %<name>-22s %<kind>s', name: name, kind: kind) }
  puts
end

puts 'The SF namespace'
puts '================'
puts
puts 'Everything the binding defines lives under the top-level SF module: five'
puts 'subsystem modules, each mirroring a namespace of SFML itself.'
puts

print_constants(SF, 'SF', 'the root: the five subsystem modules (+ VERSION)')
SUBSYSTEMS.each do |name|
  print_constants(SF.const_get(name), "SF::#{name}", NOTES.fetch(name))
end

puts 'Two ways to name the same thing'
puts '-------------------------------'
puts
puts 'The examples mix a subsystem into the top level so the names read naturally:'
puts
puts '  require "sfml"'
puts '  include SF::Window'
puts '  include SF::Graphics'
puts
puts 'Then Window means SF::Window::Window and CircleShape means'
puts 'SF::Graphics::CircleShape. The fully-qualified names always work as well;'
puts 'pick the mixin in short scripts, the qualified name in libraries.'
puts

puts "The legacy SFML alias still resolves: SFML.equal?(SF) -> #{SFML.equal?(SF)}"
puts 'New code should use SF; SFML remains so pre-rename code keeps working.'
puts

classes = SUBSYSTEMS.sum { |name| own_constants(SF.const_get(name)).count { |_, kind| kind == 'class' } }
puts "#{classes} classes across the five subsystems. See the Learn tutorials for each one."
