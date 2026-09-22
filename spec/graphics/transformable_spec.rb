# frozen_string_literal: true

require_relative '../spec_helper'

# The Transformable mixin shared by every steerable drawable, plus the
# standalone Transformable::Instance. The instance needs no display, so its
# behavior is exercised; the concrete shapes are only inspected structurally.
RSpec.describe SFML::Transformable do
  describe 'Transformable mixin' do
    it 'is a module with the full surface' do
      expect(SFML::Transformable).to be_a(Module)
      expect(SFML::Transformable).not_to be_a(Class)
      %i[position position= rotation rotation= scale scale= origin origin=
         move rotate scale! transform matrix inverse_transform].each do |name|
        expect(SFML::Transformable.instance_methods(false)).to include(name)
      end
    end

    it 'returns the standalone instance from Transformable.new' do
      instance = SFML::Transformable.new
      expect(instance).to be_instance_of(SFML::Transformable::Instance)
      expect(SFML::Transformable::Instance).to be < Object
      expect(SFML::Transformable::Instance.ancestors).to include(SFML::Transformable)
      expect(SFML::Transformable::Instance).not_to be < SFML::Shape
    end

    it 'is included by every steerable drawable' do
      [SFML::CircleShape, SFML::RectangleShape, SFML::ConvexShape, SFML::Shape,
       SFML::Sprite, SFML::Text].each do |klass|
        expect(klass.ancestors).to include(SFML::Transformable)
      end
    end

    it 'owns the transform methods' do
      expect(SFML::Shape.instance_method(:position).owner).to eq(SFML::Transformable)
      expect(SFML::Sprite.instance_method(:move).owner).to eq(SFML::Transformable)
      expect(SFML::Text.instance_method(:rotate).owner).to eq(SFML::Transformable)
    end
  end

  describe 'Transformable::Instance behavior' do
    it 'defaults to identity' do
      t = SFML::Transformable.new
      expect(t.position).to be_vec_in_epsilon([0, 0])
      expect(t.rotation).to be_within(0.01).of(0)
      expect(t.scale).to be_vec_in_epsilon([1, 1])
      expect(t.origin).to be_vec_in_epsilon([0, 0])
    end

    it 'round-trips the setters and getters' do
      t = SFML::Transformable.new
      t.position = [5, 10]
      expect(t.position).to be_vec_in_epsilon([5, 10])

      t.rotation = 45
      expect(t.rotation).to be_within(0.01).of(45)

      t.scale = [2, 3]
      expect(t.scale).to be_vec_in_epsilon([2, 3])

      t.origin = [1, 1]
      expect(t.origin).to be_vec_in_epsilon([1, 1])
    end

    it 'accumulates via move' do
      t = SFML::Transformable.new
      t.move [10, 20]
      expect(t.position).to be_vec_in_epsilon([10, 20])
      t.move [5, 5]
      expect(t.position).to be_vec_in_epsilon([15, 25])
    end

    it 'accumulates via rotate' do
      t = SFML::Transformable.new
      t.rotate 30
      expect(t.rotation).to be_within(0.01).of(30)
      t.rotate 20
      expect(t.rotation).to be_within(0.01).of(50)
    end

    it 'accumulates via scale!' do
      t = SFML::Transformable.new
      t.scale! [2, 2]
      expect(t.scale).to be_vec_in_epsilon([2, 2])
      t.scale! [3, 3]
      expect(t.scale).to be_vec_in_epsilon([6, 6])
    end

    it 'exposes a 9-element transform matrix' do
      t = SFML::Transformable.new
      matrix = t.transform
      expect(matrix).to be_a(Array)
      expect(matrix.length).to eq(9)
      expect(t.matrix).to eq(matrix)
    end

    it 'inverts its transform' do
      t = SFML::Transformable.new
      t.position = [10, 20]

      inverse = t.inverse_transform
      expect(inverse.length).to eq(9)
      # The inverse must undo the transform, which is what makes it usable for
      # turning a world point back into local space.
      expect((SFML::Transform.from_a(t.transform) * SFML::Transform.from_a(inverse)).to_a)
        .to be_matrix_in_delta(SFML::Transform.identity.to_a)
    end

    it 'copies independently' do
      t = SFML::Transformable.new
      t.position = [3, 4]

      copy = t.copy
      expect(copy.position).to be_vec_in_epsilon([3, 4])

      copy.position = [9, 9]
      expect(t.position).to be_vec_in_epsilon([3, 4])
    end
  end

  describe 'Transformable dispatch to concrete shapes' do
    it 'routes the transform methods to each shape' do
      shapes = [SFML::CircleShape.new(10), SFML::RectangleShape.new, SFML::ConvexShape.new(4)]

      shapes.each do |shape|
        shape.position = [3.5, 7.2]
        expect(shape.position).to be_vec_in_epsilon([3.5, 7.2], 0.01)
        shape.rotation = 33
        expect(shape.rotation).to be_within(0.01).of(33)
        shape.scale = [2, 5]
        expect(shape.scale).to be_vec_in_epsilon([2, 5], 0.01)
        shape.origin = [1, 1]
        expect(shape.origin).to be_vec_in_epsilon([1, 1], 0.01)
      end
    end

    it 'keeps per-shape transform state independent' do
      a = SFML::CircleShape.new(1)
      b = SFML::RectangleShape.new
      a.position = [1, 1]
      b.position = [9, 9]
      expect(a.position).to be_vec_in_epsilon([1, 1])
      expect(b.position).to be_vec_in_epsilon([9, 9])
    end
  end
end
