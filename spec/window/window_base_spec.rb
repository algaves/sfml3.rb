# frozen_string_literal: true

require_relative '../spec_helper'

# WindowBase/Window/RenderWindow inheritance. Structure only: constructing any
# of these needs a display (SFML aborts the process rather than raising), so
# nothing here is instantiated.
RSpec.describe SF::Window::WindowBase do
  it 'is the root of the window hierarchy' do
    expect(SF::Window::Window).to be < SF::Window::WindowBase
    expect(SF::Graphics::RenderWindow).to be < SF::Window::Window
    expect(SF::Window::WindowBase).not_to be < SF::Window::Window
    expect(SF::Window::Window).not_to be < SF::Graphics::RenderWindow

    expect(SF::Window::Window.superclass).to eq(SF::Window::WindowBase)
    expect(SF::Graphics::RenderWindow.superclass).to eq(SF::Window::Window)
    expect(SF::Window::WindowBase.superclass).to eq(Object)
  end

  it 'owns every shared window method' do
    %i[is_open? close! poll_event! wait_event! position size focus? native_handle
       position= size= minimum_size= maximum_size= title= set_icon visible=
       cursor_visible= cursor_grabbed= mouse_cursor= cursor= key_repeat_enabled=
       joystick_threshold= request_focus create_vulkan_surface].each do |name|
      expect(SF::Window::WindowBase.instance_methods).to include(name)
    end
  end

  it 'keeps the Window-only methods off WindowBase' do
    # :display is deliberately left out: Object#display exists, so it would
    # make the WindowBase assertion meaningless.
    %i[clear active= settings frame_rate= vertical_sync_enabled=].each do |name|
      expect(SF::Window::Window.instance_methods(false)).to include(name)
      expect(SF::Window::WindowBase.instance_methods(false)).not_to include(name)
    end
  end

  it 'lets Window redefine the shared methods while RenderWindow inherits them' do
    # ext/window/window_base.inc is compiled once per class, so Window owns its
    # own copies; RenderWindow does not include it and reuses Window's.
    expect(SF::Window::WindowBase.instance_method(:is_open?).owner).to eq(SF::Window::WindowBase)
    expect(SF::Window::Window.instance_method(:is_open?).owner).to eq(SF::Window::Window)
    expect(SF::Graphics::RenderWindow.instance_method(:is_open?).owner).to eq(SF::Window::Window)
  end
end
