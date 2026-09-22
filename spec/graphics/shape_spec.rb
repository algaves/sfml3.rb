# frozen_string_literal: true

require_relative '../spec_helper'

# Shape inheritance: SFML::Shape is the abstract base of Circle/Rectangle/
# Convex and drives concrete subclasses through the #point_count / #point
# callbacks. Shapes and the standalone Transformable::Instance need no display.
RSpec.describe SFML::Shape do
  describe 'Shape hierarchy' do
    it 'makes Shape the base of the built-in shapes' do
      expect(SFML::CircleShape).to be < SFML::Shape
      expect(SFML::RectangleShape).to be < SFML::Shape
      expect(SFML::ConvexShape).to be < SFML::Shape

      expect(SFML::CircleShape.superclass).to eq(SFML::Shape)
      expect(SFML::RectangleShape.superclass).to eq(SFML::Shape)
      expect(SFML::ConvexShape.superclass).to eq(SFML::Shape)
      expect(SFML::Shape.superclass).to eq(Object)
    end

    it 'keeps the built-in shapes as siblings' do
      expect(SFML::CircleShape).not_to be < SFML::RectangleShape
      expect(SFML::RectangleShape).not_to be < SFML::ConvexShape
      expect(SFML::ConvexShape).not_to be < SFML::CircleShape
    end

    it 'keeps Sprite and Text out of the shape hierarchy' do
      expect(SFML::Sprite).not_to be < SFML::Shape
      expect(SFML::Text).not_to be < SFML::Shape
      expect(SFML::Sprite).not_to be < SFML::Text
    end

    it 'keeps update! off the concrete shapes' do
      expect(SFML::Shape.instance_methods).to include(:update!)
      [SFML::CircleShape, SFML::RectangleShape, SFML::ConvexShape].each do |klass|
        expect(klass.instance_methods).not_to include(:update!)
      end
    end
  end

  it 'drives a custom Shape subclass through its callbacks' do
    shape_class = Class.new(SFML::Shape) do
      def point_count
        3
      end

      def point(index)
        [[0, 0], [10, 0], [0, 10]][index]
      end
    end

    shape = shape_class.new
    shape.update!
    expect(shape.local_bounds).to be_vec_in_epsilon([0, 0, 10, 10])
  end

  describe 'Bracket constructors' do
    it 'build shapes' do
      circle = SFML::CircleShape[30, [100, 100]]

      expect(circle.radius).to be_within(1e-5).of(30)
      expect(circle.position.to_a).to be_vec_in_epsilon([100, 100])
      expect(SFML::CircleShape[5].position.to_a).to be_vec_in_epsilon([0, 0])

      rectangle = SFML::RectangleShape[10, 20, 100, 40]
      expect(rectangle.position.to_a).to be_vec_in_epsilon([10, 20])
      expect(rectangle.size.to_a).to be_vec_in_epsilon([100, 40])

      polygon = SFML::ConvexShape[[0, 0], [100, 0], [50, 80]]
      expect(polygon.point_count).to eq(3)
      expect(polygon.point(1).to_a).to be_vec_in_epsilon([100, 0])
    end
  end
end
