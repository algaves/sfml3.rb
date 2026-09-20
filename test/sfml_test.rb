# frozen_string_literal: true

require_relative 'test_helper'
require 'stringio'
require 'tmpdir'
require 'fileutils'
require 'timeout'

class SfmlTest < Minitest::Test
  include SFML
  include SFMLTestHelpers

  # Text needs a real font file and CI images do not all ship one, so every
  # font-dependent test skips rather than fails when none is found.
  def system_font_path
    candidates = [
      '/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',
      '/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf',
      '/usr/share/fonts/google-noto/NotoSans-Regular.ttf',
      '/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf',
      '/Library/Fonts/Arial.ttf'
    ]

    candidates.find { |path| File.exist?(path) } ||
      Dir.glob('/usr/share/fonts/**/*.ttf').first
  end

  # Clock
  def test_elapsed_time_nonnegative
    clock = Clock.new
    elapsed = clock.elapsed_time
    assert_kind_of Time, elapsed
    assert elapsed.as_seconds >= 0, "elapsed_time must be >= 0, got #{elapsed}"
  end

  def test_restart_resets
    clock = Clock.new
    t1 = clock.elapsed_time.as_seconds
    t2 = clock.restart!.as_seconds
    assert t2 >= 0, "restart! elapsed must be >= 0, got #{t2}"
    # restart returns previous elapsed and resets near zero
    assert t2 <= t1 + 0.01, "restart! should reset near zero, got #{t2} vs #{t1}"
  end

  def test_clock_running_state
    clock = Clock.new
    assert clock.running?
    clock.stop!
    refute clock.running?
    clock.start!
    assert clock.running?
  end

  def test_clock_copy
    clock = Clock.new
    copy = clock.copy
    assert_kind_of Clock, copy
  end

  # Time
  def test_time_constructors
    assert_in_epsilon 1.5, Time.seconds(1.5).as_seconds, 0.001
    assert_equal 1500, Time.milliseconds(1500).as_milliseconds
    assert_equal 2500, Time.microseconds(2500).as_microseconds
    assert_equal 0, Time.zero.as_microseconds
  end

  def test_time_arithmetic_and_comparison
    a = Time.seconds(2)
    b = Time.seconds(0.5)

    assert_in_epsilon 2.5, (a + b).as_seconds, 0.001
    assert_in_epsilon 1.5, (a - b).as_seconds, 0.001
    assert_in_epsilon 4.0, (a * 2).as_seconds, 0.001
    assert a > b
    assert a == Time.seconds(2)
  end

  # Vector2
  def test_vector2_construct_and_query
    v = Vector2.new(3, 4)
    assert_in_epsilon 3, v.x, 0.001
    assert_in_epsilon 4, v.y, 0.001
    assert_equal [3.0, 4.0], v.to_a
  end

  def test_vector2_accepts_array_and_equality
    assert_equal Vector2.new(1, 2), Vector2.new([1, 2])
    refute_equal Vector2.new(1, 2), Vector2.new(2, 1)
    assert_equal [1.0, 2.0], Vector2.new(1, 2).to_a
  end

  def test_vector2_arithmetic
    assert_equal [4.0, 6.0], (Vector2.new(1, 2) + [3, 4]).to_a
    assert_equal [-2.0, -2.0], (Vector2.new(1, 2) - [3, 4]).to_a
    assert_equal [2.0, 4.0], (Vector2.new(1, 2) * 2).to_a
  end

  # Vector3
  def test_vector3
    v = Vector3.new(1, 2, 3)
    assert_in_epsilon 1, v.x, 0.001
    assert_in_epsilon 3, v.z, 0.001
    assert_equal [1.0, 2.0, 3.0], v.to_a
    assert_equal Vector3.new([1, 2, 3]), v
  end

  # Color
  def test_color_construct
    c = Color.new(10, 20, 30, 40)
    assert_equal 10, c.r
    assert_equal 40, c.a
    assert_equal [10, 20, 30, 40], c.to_a
  end

  def test_color_array_and_integer
    assert_equal Color.new(10, 20, 30), Color.new([10, 20, 30, 255])
    assert_equal Color.new(0x11, 0x22, 0x33, 0x44), Color.new(0x11223344)
    assert_equal 0x11223344, Color.new(0x11223344).to_i
  end

  def test_color_predefined_and_operations
    assert_equal [255, 255, 255, 255], Color::WHITE.to_a
    assert_equal [0, 0, 0, 255], Color::BLACK.to_a
    assert_equal [255, 255, 255, 255], (Color.new(100, 100, 100) + Color.new(200, 200, 200)).to_a
  end

  # Rect
  def test_rect_construct_and_query
    r = Rect.new(1, 2, 10, 20)
    assert_in_epsilon 1, r.left, 0.001
    assert_in_epsilon 20, r.height, 0.001
    assert_equal [1.0, 2.0, 10.0, 20.0], r.to_a
    assert_equal Vector2.new(1, 2), r.position
    assert_equal Vector2.new(10, 20), r.size
  end

  def test_rect_contains_and_intersects
    r = Rect.new(0, 0, 10, 10)
    assert r.contains?([5, 5])
    refute r.contains?([15, 5])

    assert r.intersects?(Rect.new(5, 5, 10, 10))
    refute r.intersects?(Rect.new(20, 20, 5, 5))

    intersection = r.intersection(Rect.new(5, 5, 10, 10))
    assert_equal [5.0, 5.0, 5.0, 5.0], intersection.to_a
    assert_nil r.intersection(Rect.new(20, 20, 5, 5))
  end

  # Circle
  def test_circle_default_radius
    c = Circle.new
    assert_in_epsilon 0, c.radius, 0.01, "default Circle radius should be 0, got #{c.radius}"
  end

  def test_circle_radius_setter_getter
    c = Circle.new 10
    assert_in_epsilon 10, c.radius, 0.01, "radius should be 10, got #{c.radius}"
  end

  def test_circle_too_many_args
    assert_raises(ArgumentError) { Circle.new 1, 2 }
  end

  def test_circle_point_count_setter
    c = Circle.new 10
    c.point_count = 8
    assert_equal 8, c.point_count
  end

  # RenderState
  def test_renderstate_default_matrix_length
    rs = RenderState.new
    matrix = rs.transform
    assert_kind_of Array, matrix
    assert_equal 9, matrix.length, "matrix should have 9 elements, got #{matrix.length}"
  end

  def test_renderstate_matrix_roundtrip
    rs = RenderState.new
    rs.transform = [1, 0, 0, 0, 1, 0, 0, 0, 1]
    assert_vec_in_epsilon [1, 0, 0, 0, 1, 0, 0, 0, 1], rs.transform
    assert_equal rs.transform, rs.matrix
  end

  # Transform
  def test_transform_identity
    assert_equal [1, 0, 0, 0, 1, 0, 0, 0, 1], Transform.identity.to_a
    assert_equal Transform.identity, Transform::IDENTITY
  end

  def test_transform_identity_constant_is_frozen
    # A shared constant that mutators could edit would silently corrupt every
    # later use of it.
    assert Transform::IDENTITY.frozen?
    assert_raises(FrozenError) { Transform::IDENTITY.translate! [1, 1] }
    assert_equal [1, 0, 0, 0, 1, 0, 0, 0, 1], Transform::IDENTITY.to_a
  end

  def test_transform_from_a_roundtrip
    t = Transform.identity.translate!([4, 9]).rotate!(30)
    assert_equal t, Transform.from_a(t.to_a)
    assert_equal t.to_a, Transform.new(*t.to_a).to_a
  end

  def test_transform_translate
    t = Transform.identity.translate!([10, 20])
    assert_matrix_in_delta [1, 0, 10, 0, 1, 20, 0, 0, 1], t.to_a
    assert_vec_in_epsilon [11, 22], t.transform_point([1, 2])
  end

  def test_transform_scale_with_center
    # Scaling about (1, 1) leaves that point where it is.
    t = Transform.identity.scale!([2, 3], [1, 1])
    assert_vec_in_epsilon [1, 1], t.transform_point([1, 1])
    assert_vec_in_epsilon [3, 4], t.transform_point([2, 2])
  end

  def test_transform_rotate_quarter_turn
    point = Transform.identity.rotate!(90).transform_point([1, 0])
    assert_in_delta 0, point.x, 0.0001
    assert_in_delta 1, point.y, 0.0001
  end

  def test_transform_mutators_chain_and_return_self
    t = Transform.identity
    assert_same t, t.translate!([1, 1])
    assert_same t, t.rotate!(10)
    assert_same t, t.scale!([2, 2])
  end

  def test_transform_non_bang_forms_do_not_mutate
    t = Transform.identity
    moved = t.translate([5, 5])
    assert_equal Transform.identity, t
    refute_equal t, moved
  end

  def test_transform_inverse_cancels_itself
    t = Transform.identity.translate!([7, -3]).rotate!(25).scale!([2, 4])
    assert_matrix_in_delta Transform.identity.to_a, (t * t.inverse).to_a
  end

  def test_transform_transform_rect
    rect = Transform.identity.translate!([10, 10]).transform_rect([0, 0, 4, 6])
    assert_matrix_in_delta [10, 10, 4, 6], rect.to_a
  end

  def test_transform_gl_matrix_is_4x4
    # Distinct from #to_a: sfTransform_getMatrix fills the 16-float OpenGL form.
    matrix = Transform.identity.gl_matrix
    assert_equal 16, matrix.length
    assert_equal 9, Transform.identity.to_a.length
  end

  def test_transform_module_functions_still_take_and_return_arrays
    # Transform was a module before it was a class; both entry points stay
    # Array-in/Array-out so existing callers keep working.
    identity = [1, 0, 0, 0, 1, 0, 0, 0, 1]
    moved = [1, 0, 5, 0, 1, 0, 0, 0, 1]

    combined = Transform.combine(identity, moved)
    assert_kind_of Array, combined
    assert_matrix_in_delta moved, combined
    assert_matrix_in_delta identity, Transform.inverse(identity)
  end

  def test_renderstate_accepts_transform_and_array
    rs = RenderState.new
    t = Transform.identity.translate!([3, 4])

    rs.transform = t
    assert_matrix_in_delta t.to_a, rs.transform

    rs.transform = [1, 0, 0, 0, 1, 0, 0, 0, 1]
    assert_matrix_in_delta [1, 0, 0, 0, 1, 0, 0, 0, 1], rs.transform
  end

  # View -- safe headless: sfView is plain data and opens no GL context, unlike
  # Window/Texture/RenderTexture.
  def test_view_from_rect
    # The rect is the visible area, so its centre is the view's centre.
    view = View.from_rect [0, 0, 100, 50]
    assert_vec_in_epsilon [50, 25], view.center
    assert_vec_in_epsilon [100, 50], view.size
  end

  def test_view_scissor_roundtrip
    view = View.new
    assert_vec_in_epsilon [0, 0, 1, 1], view.scissor.to_a

    view.scissor = [0.25, 0.25, 0.5, 0.5]
    assert_vec_in_epsilon [0.25, 0.25, 0.5, 0.5], view.scissor.to_a
  end

  # Text
  def test_text_string_roundtrips_non_ascii
    path = system_font_path
    skip 'no system font available' unless path

    text = Text.new Font.from_file(path)

    # Goes through sfText_setUnicodeString: the plain char* entry point decodes
    # the bytes with the C locale and turns each non-ASCII byte into U+FFFFFFFF.
    ['hello', 'héllo', '日本語', 'emoji 🎮', ''].each do |string|
      text.string = string
      assert_equal string, text.string
      assert_equal Encoding::UTF_8, text.string.encoding
    end
  end

  # VideoMode
  def test_videomode_construct
    vm = VideoMode.new 800, 600, 32
    assert_kind_of Integer, vm.width
    assert_kind_of Integer, vm.height
    assert_kind_of Integer, vm.bits
  end

  # Sleep, Buffer, InputStream
  def test_sleep_accepts_time_and_number
    assert_in_epsilon 0.0, SFML.sleep(0), 0.001
    assert_kind_of Time, SFML.sleep(Time.zero)
  end

  def test_buffer_new_is_empty
    buffer = Buffer.new
    assert buffer.empty?
    assert_equal 0, buffer.size
    assert_equal '', buffer.data
  end

  def test_input_stream_wraps_io
    io = StringIO.new('hello')
    stream = InputStream.new(io)
    assert_same io, stream.io
  end

  def test_input_stream_requires_read
    assert_raises(ArgumentError) { InputStream.new(Object.new) }
  end

  # BlendMode / StencilMode
  def test_blend_mode_predefined
    assert_equal :dst_color, BlendMode::MULTIPLY.color_src_factor
    assert_equal :add, BlendMode::MULTIPLY.color_equation
    assert_equal :one, BlendMode::ALPHA.alpha_src_factor
  end

  def test_blend_mode_custom
    mode = BlendMode.new(:src_alpha, :one_minus_src_alpha, :add, :one, :zero, :add)
    assert_equal :src_alpha, mode.color_src_factor
    assert_equal :zero, mode.alpha_dst_factor
  end

  def test_stencil_mode
    mode = StencilMode.new(:equal, :replace, 3, 0xFF, true)
    assert_equal :equal, mode.comparison
    assert_equal :replace, mode.update_operation
    assert_equal 3, mode.reference
    assert_equal 0xFF, mode.mask
    assert mode.stencil_only
  end

  # RenderState extras
  def test_renderstate_blend_and_stencil
    rs = RenderState.new
    rs.blend_mode = BlendMode::NONE
    assert_equal :one, rs.blend_mode.color_src_factor

    rs.stencil_mode = StencilMode.new(:always, :keep, 0, 0, false)
    assert_equal :always, rs.stencil_mode.comparison

    rs.coordinate_type = :pixels
    assert_equal :pixels, rs.coordinate_type
  end

  # ContextSettings
  def test_context_settings
    settings = ContextSettings.new(24, 8, 4, 3, 3, [:core], false)
    assert_equal 24, settings.depth_bits
    assert_equal 4, settings.antialiasing_level
    assert_equal [:core], settings.attribute_flags
    refute settings.srgb_capable?

    settings.attribute_flags = :debug
    assert_equal [:debug], settings.attribute_flags
  end

  # Keyboard
  def test_keyboard_key_mapping
    assert_equal 0, Keyboard.delocalize(:a)
    assert_equal :e, Keyboard.localize(4)
    assert_equal 0, Keyboard.delocalize('a')
  end

  def test_keyboard_unknown_key
    assert_raises(ArgumentError) { Keyboard.delocalize(:not_a_key) }
  end

  # Image
  def test_image_from_color_and_pixel
    image = Image.from_color([4, 4], Color.new(10, 20, 30, 255))
    assert_equal [4.0, 4.0], image.size.to_a
    assert_equal [10, 20, 30, 255], image.pixel(1, 1).to_a
    assert_equal 4 * 4 * 4, image.pixels.bytesize
  end

  def test_image_set_pixel_and_copy
    image = Image.new([2, 2])
    image.set_pixel(0, 0, Color::RED)
    assert_equal [255, 0, 0, 255], image.pixel(0, 0).to_a

    copy = image.copy
    assert_equal image.pixel(0, 0), copy.pixel(0, 0)
  end

  def test_image_from_pixels
    pixels = [255, 0, 0, 255, 0, 255, 0, 255].pack('C*')
    image = Image.from_pixels([2, 1], pixels)
    assert_equal [255, 0, 0, 255], image.pixel(0, 0).to_a
    assert_equal [0, 255, 0, 255], image.pixel(1, 0).to_a
  end

  def test_image_save_to_memory
    image = Image.from_color([2, 2], Color::WHITE)
    buffer = image.save_to_memory
    assert_kind_of Buffer, buffer
    refute buffer.empty?
  end

  # Vertex / VertexArray
  def test_vertex
    vertex = Vertex.new([1, 2], Color::WHITE, [0.5, 0.25])
    assert_vec_in_epsilon [1, 2], vertex.position
    assert_equal [255, 255, 255, 255], vertex.color.to_a
    assert_vec_in_epsilon [0.5, 0.25], vertex.tex_coords
  end

  def test_vertex_array
    array = VertexArray.new
    array.append(Vertex.new([0, 0]))
    array.append([10, 0])
    array.primitive = :triangles

    assert_equal 2, array.vertex_count
    assert_equal :triangles, array.primitive
    assert_equal [10.0, 0.0], array.vertex(1).position.to_a
  end

  # Shapes
  def test_rectangle_shape
    shape = RectangleShape.new([10, 20])
    assert_vec_in_epsilon [10, 20], shape.size
    assert_equal 4, shape.point_count
    assert_in_epsilon 200, shape.local_bounds.size.to_a.reduce(:*), 0.01
  end

  def test_convex_shape
    shape = ConvexShape.new(3)
    shape.set_point 0, [0, 0]
    shape.set_point 1, [10, 0]
    shape.set_point 2, [0, 10]
    assert_equal 3, shape.point_count
    assert_vec_in_epsilon [10, 0], shape.point(1)
  end

  def test_custom_shape_callbacks
    shape_class = Class.new(Shape) do
      def point_count
        3
      end

      def point(index)
        [[0, 0], [10, 0], [0, 10]][index]
      end
    end

    shape = shape_class.new
    shape.update!
    assert_vec_in_epsilon [0, 0, 10, 10], shape.local_bounds
  end

  # Cursor / Clipboard / Vulkan
  def test_cursor_from_system
    assert_kind_of Cursor, Cursor.from_system(:hand)
  end

  def test_clipboard_roundtrip
    Clipboard.string = 'sfml clipboard'
    assert_equal 'sfml clipboard', Clipboard.string
  end

  def test_vulkan_available_is_boolean
    assert_includes [true, false], Vulkan.available?
  end

  # --- Audio -----------------------------------------------------------------

  def test_audio_enum_constants
    assert_equal 0, SoundStatus::STOPPED
    assert_equal 2, SoundStatus::PLAYING
    assert_equal 0, SoundChannel::UNSPECIFIED
    assert_kind_of Integer, SoundChannel::FRONT_LEFT
  end

  def test_sound_buffer_from_samples
    buffer = SoundBuffer.from_samples([0, 100, -100, 0], 1, 44_100)

    assert_equal 4, buffer.sample_count
    assert_equal 44_100, buffer.sample_rate
    assert_equal 1, buffer.channel_count
    assert_equal [0, 100, -100, 0], buffer.samples
    assert_equal [:mono], buffer.channel_map
    assert_in_delta 4.0 / 44_100, buffer.duration.as_seconds, 0.0001
  end

  def test_sound_buffer_from_packed_string
    packed = [10, -10, 20, -20].pack('s<*')
    buffer = SoundBuffer.from_samples(packed, 1, 22_050)

    assert_equal 4, buffer.sample_count
    assert_equal [10, -10, 20, -20], buffer.samples
  end

  def test_sound_buffer_copy_and_save
    buffer = SoundBuffer.from_samples([1, 2, 3, 4], 1, 8000)
    copy = buffer.copy
    assert_equal buffer.sample_count, copy.sample_count

    path = File.join(Dir.tmpdir, "sfml3_rb_test_#{Process.pid}.wav")
    begin
      assert buffer.save_to_file(path), 'save_to_file should succeed'
      reloaded = SoundBuffer.from_file(path)
      assert_equal buffer.sample_count, reloaded.sample_count
      assert_equal buffer.sample_rate, reloaded.sample_rate
    ensure
      FileUtils.rm_f(path)
    end
  end

  def test_sound_buffer_from_stream
    path = File.join(Dir.tmpdir, "sfml3_rb_stream_#{Process.pid}.wav")

    begin
      SoundBuffer.from_samples([1, 2, 3, 4, 5, 6], 1, 8000).save_to_file(path)
      buffer = SoundBuffer.from_stream(File.open(path, 'rb'))
      assert_equal 6, buffer.sample_count
    ensure
      FileUtils.rm_f(path)
    end
  end

  def test_sound_source_cone
    cone = SoundSourceCone.new(10, 20, 0.5)

    assert_in_delta 10, cone.inner_angle, 0.001
    assert_in_delta 20, cone.outer_angle, 0.001
    assert_in_delta 0.5, cone.outer_gain, 0.001
    assert_equal [10, 20, 0.5], cone.to_a
    assert_equal cone, SoundSourceCone.new(10, 20, 0.5)
  end

  def test_listener_roundtrip
    Listener.global_volume = 33.0
    assert_in_delta 33.0, Listener.global_volume, 0.001

    Listener.position = [1, 2, 3]
    assert_vec_in_epsilon [1, 2, 3], Listener.position.to_a

    Listener.up_vector = [0, 1, 0]
    assert_vec_in_epsilon [0, 1, 0], Listener.up_vector.to_a

    Listener.cone = [10, 20, 0.25]
    assert_in_delta 0.25, Listener.cone.outer_gain, 0.001
  ensure
    Listener.global_volume = 100.0
    Listener.position = [0, 0, 0]
  end

  def test_sound_properties
    buffer = SoundBuffer.from_samples([0, 0, 0, 0], 1, 44_100)
    sound = Sound.new(buffer)

    sound.volume = 0.25
    sound.pitch = 1.5
    sound.pan = -0.5
    sound.position = [4, 5, 6]
    sound.cone = SoundSourceCone.new(90, 180, 0.1)

    assert_in_delta 0.25, sound.volume, 0.001
    assert_in_delta 1.5, sound.pitch, 0.001
    assert_in_delta(-0.5, sound.pan, 0.001)
    assert_vec_in_epsilon [4, 5, 6], sound.position.to_a
    assert_equal :stopped, sound.status

    copy = sound.copy
    assert_in_delta 0.25, copy.volume, 0.001
  end

  def test_sound_effect_processor_assignment
    buffer = SoundBuffer.from_samples([0, 0], 1, 44_100)
    sound = Sound.new(buffer)

    sound.effect_processor = proc { |frames, _channels| frames }
    sound.effect_processor = nil
    assert_kind_of Sound, sound
  end

  def test_music_from_file
    path = File.join(Dir.tmpdir, "sfml3_rb_music_#{Process.pid}.wav")

    begin
      SoundBuffer.from_samples(Array.new(200) { |i| (i % 100) - 50 }, 2, 44_100).save_to_file(path)
      music = Music.from_file(path)

      assert_equal 2, music.channel_count
      assert_equal 44_100, music.sample_rate
      assert_operator music.duration.as_seconds, :>, 0
      assert_equal :stopped, music.status
    ensure
      FileUtils.rm_f(path)
    end
  end

  def test_sound_stream_subclass
    klass = Class.new(SoundStream) do
      def on_get_data
        [0, 0]
      end
    end

    stream = klass.new(1, 44_100, [:mono])

    assert_equal 1, stream.channel_count
    assert_equal 44_100, stream.sample_rate
    assert_equal :stopped, stream.status
  end

  def test_sound_stream_requires_on_get_data
    assert_raises(NotImplementedError) { SoundStream.new(1, 44_100, [:mono]) }
  end

  def test_sound_recorder_availability
    assert_includes [true, false], SoundRecorder.available?
    assert_kind_of Array, SoundRecorder.available_devices

    # A runner with no capture hardware reports no default device; CSFML
    # returns NULL and the binding maps that to nil rather than raising.
    assert_includes [String, NilClass], SoundRecorder.default_device.class
  end

  # --- Network ---------------------------------------------------------------

  def test_socket_status_constants
    assert_equal 0, SocketStatus::DONE
    assert_equal 4, SocketStatus::ERROR
  end

  def test_ftp_and_http_enum_constants
    assert_equal 200, HttpStatus::OK
    assert_equal 0, HttpMethod::GET
    assert_equal 0, FtpTransferMode::BINARY
    assert_equal 200, FtpStatus::OK
  end

  def test_ip_address_roundtrip
    address = IpAddress.from_string('127.0.0.1')

    assert_equal '127.0.0.1', address.to_s
    assert_equal 2_130_706_433, address.to_integer
    assert_equal address, IpAddress.from_bytes(127, 0, 0, 1)
    assert_equal address, IpAddress.from_integer(address.to_integer)
    refute_equal address, IpAddress::ANY
  end

  def test_ip_address_constants
    assert_equal '0.0.0.0', IpAddress::ANY.to_s
    assert_equal '255.255.255.255', IpAddress::BROADCAST.to_s
    assert_equal '127.0.0.1', IpAddress::LOCAL_HOST.to_s
    assert_kind_of IpAddress, IpAddress.local_address
  end

  def test_packet_roundtrip
    packet = Packet.new
    packet.write_bool(true)
    packet.write_int8(-8)
    packet.write_uint8(200)
    packet.write_int16(-32_000)
    packet.write_uint16(60_000)
    packet.write_int32(-2_000_000_000)
    packet.write_uint32(4_000_000_000)
    packet.write_int64(-9_000_000_000)
    packet.write_uint64(18_000_000_000)
    packet.write_float(1.5)
    packet.write_double(2.5)
    packet.write_string('packet payload')

    copy = packet.copy

    assert_equal true, copy.read_bool
    assert_equal(-8, copy.read_int8)
    assert_equal 200, copy.read_uint8
    assert_equal(-32_000, copy.read_int16)
    assert_equal 60_000, copy.read_uint16
    assert_equal(-2_000_000_000, copy.read_int32)
    assert_equal 4_000_000_000, copy.read_uint32
    assert_equal(-9_000_000_000, copy.read_int64)
    assert_equal 18_000_000_000, copy.read_uint64
    assert_in_delta 1.5, copy.read_float, 0.0001
    assert_in_delta 2.5, copy.read_double, 0.0001
    assert_equal 'packet payload', copy.read_string
  end

  def test_packet_raw_data_and_append
    packet = Packet.new
    packet.append('abcd')
    packet.append("\x00\x01")

    assert_equal 6, packet.data_size
    assert_equal "abcd\x00\x01".b, packet.data.b
  end

  def test_tcp_loopback
    listener = TcpListener.new
    assert_equal :done, listener.listen(0)

    client = TcpSocket.new
    assert_equal :done, Timeout.timeout(5) { client.connect('127.0.0.1', listener.local_port) }

    connection, status = Timeout.timeout(5) { listener.accept }
    assert_equal :done, status
    assert_kind_of TcpSocket, connection

    assert_equal :done, client.send('ping')
    data, receive_status = Timeout.timeout(5) { connection.receive(64) }

    assert_equal :done, receive_status
    assert_equal 'ping', data
  end

  def test_udp_loopback
    socket = UdpSocket.new
    assert_equal :done, socket.bind(0)

    port = socket.local_port
    assert_equal :done, socket.send('pong', '127.0.0.1', port)

    data, address, remote_port, status = Timeout.timeout(5) { socket.receive(64) }

    assert_equal :done, status
    assert_equal 'pong', data
    assert_equal '127.0.0.1', address.to_s
    assert_equal port, remote_port
  end

  def test_socket_selector_waits_for_connection
    listener = TcpListener.new
    listener.listen(0)

    selector = SocketSelector.new
    selector.add(listener)

    # A zero timeout means "block forever" in SFML, so an explicit tiny
    # timeout is what expresses a non-blocking poll.
    assert_equal false, selector.wait(SFML::Time.microseconds(1))

    client = TcpSocket.new
    Timeout.timeout(5) { client.connect('127.0.0.1', listener.local_port) }

    assert Timeout.timeout(5) { selector.wait(SFML::Time.seconds(2)) }
    assert selector.tcp_listener_ready?(listener)

    selector.remove(listener)
    selector.clear
  end

  def test_http_request_configuration
    assert_kind_of Http, Http.new

    request = HttpRequest.new
    request.method = :post
    request.uri = '/index'
    request.set_http_version(1, 1)
    request.body = 'payload'
    request.set_field('Accept', '*/*')

    assert_kind_of HttpRequest, request
  end

  def test_ftp_construction
    assert_kind_of Ftp, Ftp.new
  end

  # Ruby warns (and undefines #allocate) whenever a T_DATA class is wrapped
  # without an allocator. Constructing every display-free class in a child
  # process catches a binding that forgot rb_define_alloc_func; the graphics
  # classes that open a context cannot be built here without a display.
  def test_no_allocator_warnings_on_construction
    lib = File.expand_path('../lib', __dir__)
    script = <<~RUBY
      require 'sfml'
      SFML::Color.new(1, 2, 3)
      SFML::BlendMode.new
      SFML::IpAddress.new(1)
      SFML::Vector2.new(1, 2)
      SFML::Vector3.new(1, 2, 3)
      SFML::Time.new(1)
      SFML::Clock.new
      SFML::View.new
      SFML::Event.new
      SFML::VideoMode.new(1, 2, 3)
      SFML::RenderState.new
      SFML::Transformable.new
      SFML::Circle.new(1)
    RUBY

    output = IO.popen([RbConfig.ruby, "-I#{lib}", '-e', script], err: %i[child out], &:read)

    refute_includes output, 'undefining the allocator'
  end
end
