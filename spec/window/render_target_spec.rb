# frozen_string_literal: true

require_relative '../spec_helper'

# The RenderTarget marker module shared by the render surfaces. Structure only:
# constructing any render target needs a display (SFML aborts the process rather
# than raising), so nothing here is instantiated.
RSpec.describe SF::Graphics::RenderTarget do
  it 'is a marker module' do
    expect(SF::Graphics::RenderTarget).to be_a(Module)
    expect(SF::Graphics::RenderTarget).not_to be_a(Class)
    # The module carries no methods: it is a marker included by the classes
    # that implement the surface via ext/graphics/render_target.inc.
    expect(SF::Graphics::RenderTarget.instance_methods(false)).to be_empty
  end

  it 'is included by the render targets but not by Window' do
    expect(SF::Graphics::RenderWindow.ancestors).to include(SF::Graphics::RenderTarget)
    expect(SF::Graphics::RenderTexture.ancestors).to include(SF::Graphics::RenderTarget)
    # Window implements the surface directly but is not a RenderTarget.
    expect(SF::Window::Window.ancestors).not_to include(SF::Graphics::RenderTarget)
  end

  it 'exposes the surface on every target' do
    %i[srgb? clear_stencil clear_color_and_stencil viewport scissor
       map_pixel_to_coords map_coords_to_pixel push_gl_states pop_gl_states
       reset_gl_states draw_primitives draw_vertex_buffer_range].each do |name|
      [SF::Window::Window, SF::Graphics::RenderWindow, SF::Graphics::RenderTexture].each do |klass|
        expect(klass.instance_methods).to include(name)
      end
    end
  end

  it 'generates the methods per class' do
    expect(SF::Window::Window.instance_method(:clear_stencil).owner).to eq(SF::Window::Window)
    expect(SF::Graphics::RenderTexture.instance_method(:clear_stencil).owner).to eq(SF::Graphics::RenderTexture)
    # RenderWindow only mixes in the empty marker module, so it keeps Window's.
    expect(SF::Graphics::RenderWindow.instance_method(:clear_stencil).owner).to eq(SF::Window::Window)
  end
end
