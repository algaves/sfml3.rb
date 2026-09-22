# frozen_string_literal: true

require_relative '../spec_helper'

# The Drawable mixin shared by every drawable class. Structure only: Sprite,
# Text, VertexArray, VertexBuffer and the shapes cannot be drawn without a
# display, so only the mixin's own behavior is exercised.
RSpec.describe SF::Graphics::Drawable do
  it 'is a module owning a default draw' do
    expect(SF::Graphics::Drawable).to be_a(Module)
    expect(SF::Graphics::Drawable).not_to be_a(Class)
    expect(SF::Graphics::Drawable.instance_methods(false)).to eq([:draw])
  end

  it 'defaults draw to a no-op' do
    drawable = Object.new.extend(SF::Graphics::Drawable)
    expect(drawable.draw(nil, nil)).to be_nil
  end

  it 'is included by every drawable' do
    [SF::Graphics::CircleShape, SF::Graphics::RectangleShape, SF::Graphics::ConvexShape, SF::Graphics::Shape,
     SF::Graphics::Sprite, SF::Graphics::Text, SF::Graphics::VertexArray, SF::Graphics::VertexBuffer].each do |klass|
      expect(klass.ancestors).to include(SF::Graphics::Drawable)
    end
  end

  it 'is absent from non-drawables' do
    [SF::System::Vector2, SF::Audio::Sound, SF::Window::WindowBase].each do |klass|
      expect(klass.ancestors).not_to include(SF::Graphics::Drawable)
    end
  end

  it 'is a module, not a superclass' do
    expect(SF::Graphics::Sprite.ancestors).to include(SF::Graphics::Drawable)
    expect(SF::Graphics::Drawable).not_to be_a(Class)
    expect(SF::Graphics::Sprite.superclass).not_to eq(SF::Graphics::Drawable)
    expect(SF::Graphics::Sprite.superclass).to eq(Object)
  end

  it 'is overridden by each concrete drawable' do
    [SF::Graphics::Sprite, SF::Graphics::Text, SF::Graphics::VertexArray, SF::Graphics::VertexBuffer,
     SF::Graphics::Shape, SF::Graphics::CircleShape].each do |klass|
      expect(klass.instance_method(:draw).owner).not_to eq(SF::Graphics::Drawable)
    end
  end
end
