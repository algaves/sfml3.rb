# frozen_string_literal: true

require_relative '../spec_helper'

# The RenderTarget marker module shared by the render surfaces. Structure only:
# constructing any render target needs a display (SFML aborts the process rather
# than raising), so nothing here is instantiated.
RSpec.describe SFML::RenderTarget do
  it 'is a marker module' do
    expect(SFML::RenderTarget).to be_a(Module)
    expect(SFML::RenderTarget).not_to be_a(Class)
    # The module carries no methods: it is a marker included by the classes
    # that implement the surface via ext/graphics/render_target.inc.
    expect(SFML::RenderTarget.instance_methods(false)).to be_empty
  end

  it 'is included by the render targets but not by Window' do
    expect(SFML::RenderWindow.ancestors).to include(SFML::RenderTarget)
    expect(SFML::RenderTexture.ancestors).to include(SFML::RenderTarget)
    # Window implements the surface directly but is not a RenderTarget.
    expect(SFML::Window.ancestors).not_to include(SFML::RenderTarget)
  end

  it 'exposes the surface on every target' do
    %i[srgb? clear_stencil clear_color_and_stencil viewport scissor
       map_pixel_to_coords map_coords_to_pixel push_gl_states pop_gl_states
       reset_gl_states draw_primitives draw_vertex_buffer_range].each do |name|
      [SFML::Window, SFML::RenderWindow, SFML::RenderTexture].each do |klass|
        expect(klass.instance_methods).to include(name)
      end
    end
  end

  it 'generates the methods per class' do
    expect(SFML::Window.instance_method(:clear_stencil).owner).to eq(SFML::Window)
    expect(SFML::RenderTexture.instance_method(:clear_stencil).owner).to eq(SFML::RenderTexture)
    # RenderWindow only mixes in the empty marker module, so it keeps Window's.
    expect(SFML::RenderWindow.instance_method(:clear_stencil).owner).to eq(SFML::Window)
  end
end
