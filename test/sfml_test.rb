require 'sfml'
require 'minitest/autorun'

class SfmlTest < Minitest::Test
  include SFML

  def assert_vec_in_epsilon(expected, actual, epsilon = 0.01)
    expected.zip(actual).each do |e, a|
      assert_in_epsilon e, a, epsilon
    end
  end

  # Clock
  def test_elapsed_time_nonnegative
    clock = Clock.new
    elapsed = clock.elapsed_time
    assert elapsed >= 0, "elapsed_time must be >= 0, got #{elapsed}"
  end

  def test_restart_resets
    clock = Clock.new
    t1 = clock.elapsed_time
    t2 = clock.restart!
    assert t2 >= 0, "restart! elapsed must be >= 0, got #{t2}"
    # restart returns previous elapsed and resets near zero
    assert t2 <= t1 + 0.01, "restart! should reset near zero, got #{t2} vs #{t1}"
  end

  # Transformable
  def test_transformable_defaults
    t = Transformable.new
    assert_vec_in_epsilon [0, 0], t.position
    assert_in_epsilon 0, t.angle, 0.01
    assert_vec_in_epsilon [1, 1], t.scale
    assert_vec_in_epsilon [0, 0], t.origin
  end

  def test_transformable_setters_getters
    t = Transformable.new
    t.position = [5, 10]
    assert_vec_in_epsilon [5, 10], t.position

    t.angle = 45
    assert_in_epsilon 45, t.angle, 0.01

    t.scale = [2, 3]
    assert_vec_in_epsilon [2, 3], t.scale

    t.origin = [1, 1]
    assert_vec_in_epsilon [1, 1], t.origin
  end

  def test_transformable_move
    t = Transformable.new
    t.move [10, 20]
    assert_vec_in_epsilon [10, 20], t.position
    t.move [5, 5]
    assert_vec_in_epsilon [15, 25], t.position
  end

  def test_transformable_rotate
    t = Transformable.new
    t.rotate 30
    assert_in_epsilon 30, t.angle, 0.01
    t.rotate 20
    assert_in_epsilon 50, t.angle, 0.01
  end

  def test_transformable_matrix_identity_length
    t = Transformable.new
    matrix = t.matrix
    assert_kind_of Array, matrix
    assert_equal 9, matrix.length, "matrix should have 9 elements (3x3), got #{matrix.length}"
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

  def test_circle_position_roundtrip
    c = Circle.new 10
    c.position = [3.5, 7.2]
    assert_vec_in_epsilon [3.5, 7.2], c.position
  end

  # RenderState
  def test_renderstate_default_matrix_length
    rs = RenderState.new
    matrix = rs.matrix
    assert_kind_of Array, matrix
    assert_equal 9, matrix.length, "matrix should have 9 elements, got #{matrix.length}"
  end

  def test_renderstate_matrix_roundtrip
    rs = RenderState.new
    rs.matrix = [1, 0, 0, 0, 1, 0, 0, 0, 1]
    assert_vec_in_epsilon [1, 0, 0, 0, 1, 0, 0, 0, 1], rs.matrix
  end

  # VideoMode
  def test_videomode_construct
    vm = VideoMode.new 800, 600, 32
    assert_kind_of Integer, vm.width
    assert_kind_of Integer, vm.height
    assert_kind_of Integer, vm.bits
  end
end