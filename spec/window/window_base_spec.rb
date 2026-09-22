# frozen_string_literal: true

require_relative '../spec_helper'

# WindowBase/Window/RenderWindow inheritance. Structure only: constructing any
# of these needs a display (SFML aborts the process rather than raising), so
# nothing here is instantiated.
RSpec.describe SFML::WindowBase do
  it 'is the root of the window hierarchy' do
    expect(SFML::Window).to be < SFML::WindowBase
    expect(SFML::RenderWindow).to be < SFML::Window
    expect(SFML::WindowBase).not_to be < SFML::Window
    expect(SFML::Window).not_to be < SFML::RenderWindow

    expect(SFML::Window.superclass).to eq(SFML::WindowBase)
    expect(SFML::RenderWindow.superclass).to eq(SFML::Window)
    expect(SFML::WindowBase.superclass).to eq(Object)
  end

  it 'owns every shared window method' do
    %i[is_open? close! poll_event! wait_event! position size focus? native_handle
       position= size= minimum_size= maximum_size= title= set_icon visible=
       cursor_visible= cursor_grabbed= mouse_cursor= cursor= key_repeat_enabled=
       joystick_threshold= request_focus create_vulkan_surface].each do |name|
      expect(SFML::WindowBase.instance_methods).to include(name)
    end
  end

  it 'keeps the Window-only methods off WindowBase' do
    # :display is deliberately left out: Object#display exists, so it would
    # make the WindowBase assertion meaningless.
    %i[clear active= settings frame_rate= vertical_sync_enabled=].each do |name|
      expect(SFML::Window.instance_methods(false)).to include(name)
      expect(SFML::WindowBase.instance_methods(false)).not_to include(name)
    end
  end

  it 'lets Window redefine the shared methods while RenderWindow inherits them' do
    # ext/window/window_base.inc is compiled once per class, so Window owns its
    # own copies; RenderWindow does not include it and reuses Window's.
    expect(SFML::WindowBase.instance_method(:is_open?).owner).to eq(SFML::WindowBase)
    expect(SFML::Window.instance_method(:is_open?).owner).to eq(SFML::Window)
    expect(SFML::RenderWindow.instance_method(:is_open?).owner).to eq(SFML::Window)
  end
end
