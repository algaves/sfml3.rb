# frozen_string_literal: true

require_relative '../spec_helper'

# The Transformable mixin shared by every steerable drawable, plus the
# standalone Transformable::Instance. The instance needs no display, so its
# behavior is exercised; the concrete shapes are only inspected structurally.
RSpec.describe SF::Graphics::Transformable do
  describe 'Transformable mixin' do
    it 'is a module with the full surface' do
      expect(SF::Graphics::Transformable).to be_a(Module)
      expect(SF::Graphics::Transformable).not_to be_a(Class)
      %i[position position= rotation rotation= scale scale= origin origin=
         move rotate scale! transform matrix inverse_transform].each do |name|
        expect(SF::Graphics::Transformable.instance_methods(false)).to include(name)
      end
    end

    it 'returns the standalone instance from Transformable.new' do
      instance = SF::Graphics::Transformable.new
      expect(instance).to be_instance_of(SF::Graphics::Transformable::Instance)
      expect(SF::Graphics::Transformable::Instance).to be < Object
      expect(SF::Graphics::Transformable::Instance.ancestors).to include(SF::Graphics::Transformable)
      expect(SF::Graphics::Transformable::Instance).not_to be < SF::Graphics::Shape
    end

    it 'is included by every steerable drawable' do
      [SF::Graphics::CircleShape, SF::Graphics::RectangleShape, SF::Graphics::ConvexShape, SF::Graphics::Shape,
       SF::Graphics::Sprite, SF::Graphics::Text].each do |klass|
        expect(klass.ancestors).to include(SF::Graphics::Transformable)
      end
    end

    it 'owns the transform methods' do
      expect(SF::Graphics::Shape.instance_method(:position).owner).to eq(SF::Graphics::Transformable)
      expect(SF::Graphics::Sprite.instance_method(:move).owner).to eq(SF::Graphics::Transformable)
      expect(SF::Graphics::Text.instance_method(:rotate).owner).to eq(SF::Graphics::Transformable)
    end
  end

  describe 'Transformable::Instance behavior' do
    it 'defaults to identity' do
      t = SF::Graphics::Transformable.new
      expect(t.position).to be_vec_in_epsilon([0, 0])
      expect(t.rotation).to be_within(0.01).of(0)
      expect(t.scale).to be_vec_in_epsilon([1, 1])
      expect(t.origin).to be_vec_in_epsilon([0, 0])
    end

    it 'round-trips the setters and getters' do
      t = SF::Graphics::Transformable.new
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
      t = SF::Graphics::Transformable.new
      t.move [10, 20]
      expect(t.position).to be_vec_in_epsilon([10, 20])
      t.move [5, 5]
      expect(t.position).to be_vec_in_epsilon([15, 25])
    end

    it 'accumulates via rotate' do
      t = SF::Graphics::Transformable.new
      t.rotate 30
      expect(t.rotation).to be_within(0.01).of(30)
      t.rotate 20
      expect(t.rotation).to be_within(0.01).of(50)
    end

    it 'accumulates via scale!' do
      t = SF::Graphics::Transformable.new
      t.scale! [2, 2]
      expect(t.scale).to be_vec_in_epsilon([2, 2])
      t.scale! [3, 3]
      expect(t.scale).to be_vec_in_epsilon([6, 6])
    end

    it 'exposes a 9-element transform matrix' do
      t = SF::Graphics::Transformable.new
      matrix = t.transform
      expect(matrix).to be_a(Array)
      expect(matrix.length).to eq(9)
      expect(t.matrix).to eq(matrix)
    end

    it 'inverts its transform' do
      t = SF::Graphics::Transformable.new
      t.position = [10, 20]

      inverse = t.inverse_transform
      expect(inverse.length).to eq(9)
      # The inverse must undo the transform, which is what makes it usable for
      # turning a world point back into local space.
      expect((SF::Graphics::Transform.from_a(t.transform) * SF::Graphics::Transform.from_a(inverse)).to_a)
        .to be_matrix_in_delta(SF::Graphics::Transform.identity.to_a)
    end

    it 'copies independently' do
      t = SF::Graphics::Transformable.new
      t.position = [3, 4]

      copy = t.copy
      expect(copy.position).to be_vec_in_epsilon([3, 4])

      copy.position = [9, 9]
      expect(t.position).to be_vec_in_epsilon([3, 4])
    end
  end

  describe 'Transformable dispatch to concrete shapes' do
    it 'routes the transform methods to each shape' do
      shapes = [SF::Graphics::CircleShape.new(10), SF::Graphics::RectangleShape.new, SF::Graphics::ConvexShape.new(4)]

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
      a = SF::Graphics::CircleShape.new(1)
      b = SF::Graphics::RectangleShape.new
      a.position = [1, 1]
      b.position = [9, 9]
      expect(a.position).to be_vec_in_epsilon([1, 1])
      expect(b.position).to be_vec_in_epsilon([9, 9])
    end
  end
end
