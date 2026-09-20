# frozen_string_literal: true

require_relative 'test_helper'

# Shape inheritance and the Drawable/Transformable mixins, plus the standalone
# Transformable::Instance. Shapes and the instance need no display, so their
# behavior is exercised; Sprite/Text are only ever inspected structurally.
class GraphicsTest < Minitest::Test
  include SFML
  include SFMLTestHelpers

  def test_shape_is_the_base_of_the_built_in_shapes
    assert_operator CircleShape, :<, Shape
    assert_operator RectangleShape, :<, Shape
    assert_operator ConvexShape, :<, Shape

    assert_equal Shape, CircleShape.superclass
    assert_equal Shape, RectangleShape.superclass
    assert_equal Shape, ConvexShape.superclass
    assert_equal Object, Shape.superclass
  end

  def test_built_in_shapes_are_siblings
    refute_operator CircleShape, :<, RectangleShape
    refute_operator RectangleShape, :<, ConvexShape
    refute_operator ConvexShape, :<, CircleShape
  end

  def test_sprite_and_text_are_not_shapes
    refute_operator Sprite, :<, Shape
    refute_operator Text, :<, Shape
    refute_operator Sprite, :<, Text
  end

  def test_circle_shape_alias
    assert_same CircleShape, Circle, 'Circle should alias the canonical CircleShape'
    assert_kind_of CircleShape, Circle.new(1)
    assert_equal 'SFML::CircleShape', CircleShape.name
  end

  def test_update_exclamation_is_only_defined_on_the_custom_shape_base
    assert_includes Shape.instance_methods, :update!
    [CircleShape, RectangleShape, ConvexShape].each do |klass|
      refute_includes klass.instance_methods, :update!, "#{klass} should not expose update!"
    end
  end

  # --- Drawable mixin --------------------------------------------------------

  def test_drawable_is_a_module_owning_a_default_draw
    assert_kind_of Module, Drawable
    refute_kind_of Class, Drawable
    assert_equal [:draw], Drawable.instance_methods(false)
  end

  def test_drawable_default_draw_is_a_no_op
    drawable = Object.new.extend(Drawable)
    assert_nil drawable.draw(nil, nil)
  end

  def test_every_drawable_includes_the_drawable_mixin
    [CircleShape, RectangleShape, ConvexShape, Shape, Sprite, Text, VertexArray, VertexBuffer].each do |klass|
      assert_includes klass.ancestors, Drawable, "#{klass} does not include Drawable"
    end
  end

  def test_non_drawables_do_not_include_the_drawable_mixin
    [Vector2, Sound, WindowBase].each do |klass|
      refute_includes klass.ancestors, Drawable, "#{klass} should not include Drawable"
    end
  end

  def test_drawable_is_a_module_not_a_superclass
    assert_includes Sprite.ancestors, Drawable
    refute_kind_of Class, Drawable
    refute_equal Drawable, Sprite.superclass
    assert_equal Object, Sprite.superclass
  end

  def test_concrete_drawables_override_the_default_draw
    [Sprite, Text, VertexArray, VertexBuffer, Shape, CircleShape].each do |klass|
      refute_equal Drawable, klass.instance_method(:draw).owner, "#{klass} does not override draw"
    end
  end

  # --- Transformable mixin ---------------------------------------------------

  def test_transformable_is_a_module_with_the_full_surface
    assert_kind_of Module, Transformable
    refute_kind_of Class, Transformable
    %i[position position= rotation rotation= scale scale= origin origin=
       move rotate scale! transform matrix inverse_transform].each do |name|
      assert_includes Transformable.instance_methods(false), name, "#{name} missing from Transformable"
    end
  end

  def test_transformable_new_returns_the_standalone_instance
    instance = Transformable.new
    assert_instance_of Transformable::Instance, instance
    assert_operator Transformable::Instance, :<, Object
    assert_includes Transformable::Instance.ancestors, Transformable
    refute_operator Transformable::Instance, :<, Shape
  end

  def test_steerable_drawables_include_the_transformable_mixin
    [CircleShape, RectangleShape, ConvexShape, Shape, Sprite, Text].each do |klass|
      assert_includes klass.ancestors, Transformable, "#{klass} does not include Transformable"
    end
  end

  def test_transformable_methods_are_owned_by_the_mixin
    assert_equal Transformable, Shape.instance_method(:position).owner
    assert_equal Transformable, Sprite.instance_method(:move).owner
    assert_equal Transformable, Text.instance_method(:rotate).owner
  end

  # --- behavior: the standalone instance -------------------------------------

  def test_transformable_defaults
    t = Transformable.new
    assert_vec_in_epsilon [0, 0], t.position
    assert_in_epsilon 0, t.rotation, 0.01
    assert_vec_in_epsilon [1, 1], t.scale
    assert_vec_in_epsilon [0, 0], t.origin
  end

  def test_transformable_setters_getters
    t = Transformable.new
    t.position = [5, 10]
    assert_vec_in_epsilon [5, 10], t.position

    t.rotation = 45
    assert_in_epsilon 45, t.rotation, 0.01

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
    assert_in_epsilon 30, t.rotation, 0.01
    t.rotate 20
    assert_in_epsilon 50, t.rotation, 0.01
  end

  def test_transformable_scale_offset
    t = Transformable.new
    t.scale! [2, 2]
    assert_vec_in_epsilon [2, 2], t.scale
    t.scale! [3, 3]
    assert_vec_in_epsilon [6, 6], t.scale
  end

  def test_transformable_matrix_identity_length
    t = Transformable.new
    matrix = t.transform
    assert_kind_of Array, matrix
    assert_equal 9, matrix.length, "matrix should have 9 elements (3x3), got #{matrix.length}"
    assert_equal matrix, t.matrix
  end

  def test_transformable_inverse_transform
    t = Transformable.new
    t.position = [10, 20]

    inverse = t.inverse_transform
    assert_equal 9, inverse.length
    # The inverse must undo the transform, which is what makes it usable for
    # turning a world point back into local space.
    assert_matrix_in_delta Transform.identity.to_a,
                           (Transform.from_a(t.transform) * Transform.from_a(inverse)).to_a
  end

  def test_transformable_copy_is_independent
    t = Transformable.new
    t.position = [3, 4]

    copy = t.copy
    assert_vec_in_epsilon [3, 4], copy.position

    copy.position = [9, 9]
    assert_vec_in_epsilon [3, 4], t.position
  end

  # --- behavior: the mixin dispatches to each concrete shape -----------------

  def test_transform_methods_dispatch_to_each_shape
    shapes = [CircleShape.new(10), RectangleShape.new, ConvexShape.new(4)]

    shapes.each do |shape|
      klass = shape.class
      shape.position = [3.5, 7.2]
      assert_vec_in_epsilon [3.5, 7.2], shape.position, 0.01
      shape.rotation = 33
      assert_in_epsilon 33, shape.rotation, 0.01, "#{klass} did not keep its rotation"
      shape.scale = [2, 5]
      assert_vec_in_epsilon [2, 5], shape.scale, 0.01
      shape.origin = [1, 1]
      assert_vec_in_epsilon [1, 1], shape.origin, 0.01
    end
  end

  def test_shapes_do_not_share_transform_state
    a = CircleShape.new(1)
    b = RectangleShape.new
    a.position = [1, 1]
    b.position = [9, 9]
    assert_vec_in_epsilon [1, 1], a.position
    assert_vec_in_epsilon [9, 9], b.position
  end
end
