# frozen_string_literal: true

# SFML::Shader: GLSL vertex and fragment programs, loaded from source with
# `Shader.from_memory`. A full-screen rectangle is drawn through a RenderState
# whose shader is set, and the shader's uniforms are updated each frame with
# `set_float` / `set_vec2`. The fragment shader here is a moving cosine palette
# with a soft spotlight that follows the mouse. If the driver exposes no
# shaders (`Shader.available?` false) the window says so and does nothing.
# Escape quits.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/glsl.rb
# (headless: SFML_EXAMPLE_FRAMES=120 xvfb-run -a bundle exec ruby -Ilib ...)

require 'sfml'
require_relative '../support'
include SFML

VERTEX = <<~GLSL
  void main()
  {
      gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
      gl_FrontColor = gl_Color;
  }
GLSL

FRAGMENT = <<~GLSL
  uniform float u_time;
  uniform vec2  u_resolution;
  uniform vec2  u_mouse;

  void main()
  {
      vec2 uv = gl_FragCoord.xy / u_resolution;
      vec2 p  = uv * 2.0 - 1.0;
      p.x *= u_resolution.x / u_resolution.y;
      vec2 m = u_mouse / u_resolution * 2.0 - 1.0;
      m.x *= u_resolution.x / u_resolution.y;

      vec3 palette = 0.5 + 0.5 * cos(u_time + uv.xyx + vec3(0.0, 2.0, 4.0));
      float spot = smoothstep(0.55, 0.0, length(p - m));
      gl_FragColor = vec4(palette * (0.35 + spot), 1.0);
  }
GLSL

window = Window.new(VideoMode.new(640, 480, 32), 'SFML GLSL')
window.frame_rate = 60
available = Shader.available?
shader = available ? Shader.from_memory(VERTEX, FRAGMENT) : nil
state = RenderState.new
state.shader = shader if shader
clock = Clock.new
canvas = RectangleShape.new(window.size)
frame = 0

loop do
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      window.close! if event.code == :escape
    end
  end

  window.clear!([12, 12, 18, 255])

  if shader
    shader.set_float('u_time', clock.elapsed_time.as_seconds)
    shader.set_vec2('u_resolution', window.size.to_a)
    shader.set_vec2('u_mouse', Mouse.position(window).to_a)
    window.draw(canvas, state)
    window.draw(ExampleSupport.text('fragment shader: animated palette + mouse spotlight',
                                    size: 14, color: [255, 255, 255, 200]))
  else
    window.draw(ExampleSupport.text(
                  "Shaders are not available on this driver\n" \
                  "(Shader.available? -> false, geometry: #{Shader.geometry_available?})\n" \
                  'escape quits', size: 18
                ))
  end

  window.display!
  frame += 1
  break if ExampleSupport.auto_close?(frame)
end
