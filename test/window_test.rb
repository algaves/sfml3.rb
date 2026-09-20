# frozen_string_literal: true

require_relative 'test_helper'

# WindowBase/Window/RenderWindow inheritance and the RenderTarget mixin they
# share. Structure only: constructing any of these needs a display (SFML aborts
# the process rather than raising), so nothing here is instantiated.
class WindowTest < Minitest::Test
  include SFML
  include SFMLTestHelpers

  def test_window_base_is_the_root_of_the_window_hierarchy
    assert_operator Window, :<, WindowBase
    assert_operator RenderWindow, :<, Window
    refute_operator WindowBase, :<, Window
    refute_operator Window, :<, RenderWindow

    assert_equal WindowBase, Window.superclass
    assert_equal Window, RenderWindow.superclass
    assert_equal Object, WindowBase.superclass
  end

  def test_window_base_methods_live_on_the_root
    %i[is_open? close! poll_event! wait_event! position size focus? native_handle
       position= size= minimum_size= maximum_size= title= set_icon visible=
       cursor_visible= cursor_grabbed= mouse_cursor= cursor= key_repeat_enabled=
       joystick_threshold= request_focus create_vulkan_surface].each do |name|
      assert_includes WindowBase.instance_methods, name, "#{name} missing from WindowBase"
    end
  end

  def test_window_only_methods_are_absent_from_window_base
    # :display is deliberately left out: Object#display exists, so it would
    # make the WindowBase assertion meaningless.
    %i[clear active= settings frame_rate= vertical_sync_enabled=].each do |name|
      assert_includes Window.instance_methods(false), name, "#{name} missing from Window"
      refute_includes WindowBase.instance_methods(false), name, "#{name} leaked onto WindowBase"
    end
  end

  def test_window_redefines_the_shared_methods_while_render_window_inherits_them
    # ext/window/window_base.inc is compiled once per class, so Window owns its
    # own copies; RenderWindow does not include it and reuses Window's.
    assert_equal WindowBase, WindowBase.instance_method(:is_open?).owner
    assert_equal Window, Window.instance_method(:is_open?).owner
    assert_equal Window, RenderWindow.instance_method(:is_open?).owner
  end

  def test_render_target_is_a_marker_module
    assert_kind_of Module, RenderTarget
    refute_kind_of Class, RenderTarget
    # The module carries no methods: it is a marker included by the classes
    # that implement the surface via ext/graphics/render_target.inc.
    assert_empty RenderTarget.instance_methods(false)
  end

  def test_render_target_module_is_shared_by_render_window_and_render_texture
    assert_includes RenderWindow.ancestors, RenderTarget
    assert_includes RenderTexture.ancestors, RenderTarget
    # Window implements the surface directly but is not a RenderTarget.
    refute_includes Window.ancestors, RenderTarget
  end

  def test_render_target_surface_is_present_on_every_target
    %i[srgb? clear_stencil clear_color_and_stencil viewport scissor
       map_pixel_to_coords map_coords_to_pixel push_gl_states pop_gl_states
       reset_gl_states draw_primitives draw_vertex_buffer_range].each do |name|
      [Window, RenderWindow, RenderTexture].each do |klass|
        assert_includes klass.instance_methods, name, "#{name} missing from #{klass}"
      end
    end
  end

  def test_render_target_methods_are_generated_per_class
    assert_equal Window, Window.instance_method(:clear_stencil).owner
    assert_equal RenderTexture, RenderTexture.instance_method(:clear_stencil).owner
    # RenderWindow only mixes in the empty marker module, so it keeps Window's.
    assert_equal Window, RenderWindow.instance_method(:clear_stencil).owner
  end
end
