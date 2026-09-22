# frozen_string_literal: true

require_relative '../spec_helper'

# The Drawable mixin shared by every drawable class. Structure only: Sprite,
# Text, VertexArray, VertexBuffer and the shapes cannot be drawn without a
# display, so only the mixin's own behavior is exercised.
RSpec.describe SFML::Drawable do
  it 'is a module owning a default draw' do
    expect(SFML::Drawable).to be_a(Module)
    expect(SFML::Drawable).not_to be_a(Class)
    expect(SFML::Drawable.instance_methods(false)).to eq([:draw])
  end

  it 'defaults draw to a no-op' do
    drawable = Object.new.extend(SFML::Drawable)
    expect(drawable.draw(nil, nil)).to be_nil
  end

  it 'is included by every drawable' do
    [SFML::CircleShape, SFML::RectangleShape, SFML::ConvexShape, SFML::Shape,
     SFML::Sprite, SFML::Text, SFML::VertexArray, SFML::VertexBuffer].each do |klass|
      expect(klass.ancestors).to include(SFML::Drawable)
    end
  end

  it 'is absent from non-drawables' do
    [SFML::Vector2, SFML::Sound, SFML::WindowBase].each do |klass|
      expect(klass.ancestors).not_to include(SFML::Drawable)
    end
  end

  it 'is a module, not a superclass' do
    expect(SFML::Sprite.ancestors).to include(SFML::Drawable)
    expect(SFML::Drawable).not_to be_a(Class)
    expect(SFML::Sprite.superclass).not_to eq(SFML::Drawable)
    expect(SFML::Sprite.superclass).to eq(Object)
  end

  it 'is overridden by each concrete drawable' do
    [SFML::Sprite, SFML::Text, SFML::VertexArray, SFML::VertexBuffer,
     SFML::Shape, SFML::CircleShape].each do |klass|
      expect(klass.instance_method(:draw).owner).not_to eq(SFML::Drawable)
    end
  end
end
