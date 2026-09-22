# frozen_string_literal: true

require 'rbconfig'
require_relative 'spec_helper'

# Constructing every display-free class in a child process catches a binding
# that forgot rb_define_alloc_func. The graphics classes that open a context
# cannot be built here without a display, so the top-level (../lib) position
# keeps the repository-relative lib path intact.
RSpec.describe 'the native extension allocators' do
  it 'constructs every display-free class without allocator warnings' do
    # Ruby warns (and undefines #allocate) whenever a T_DATA class is wrapped
    # without an allocator.
    lib = File.expand_path('../lib', __dir__)
    script = <<~RUBY
      require 'sfml'
      SF::Graphics::Color.new(1, 2, 3)
      SF::Graphics::BlendMode.new
      SF::Network::IpAddress.new(1)
      SF::System::Vector2.new(1, 2)
      SF::System::Vector3.new(1, 2, 3)
      SF::System::Time.new(1)
      SF::System::Clock.new
      SF::Graphics::View.new
      SF::Window::Event.new
      SF::Window::VideoMode.new(1, 2, 3)
      SF::Graphics::RenderState.new
      SF::Graphics::Transformable.new
      SF::Graphics::Circle.new(1)
    RUBY

    output = IO.popen([RbConfig.ruby, "-I#{lib}", '-e', script], err: %i[child out], &:read)

    expect(output).not_to include('undefining the allocator')
  end
end

# Backward compatibility for the pre-`SF::` spelling: `SFML` stays an alias of
# the canonical `SF` root, so unchanged legacy code keeps resolving through it.
RSpec.describe 'the SFML compatibility alias' do
  it 'points SFML at the canonical SF namespace' do
    expect(SFML).to equal(SF)
  end

  it 'resolves the subsystem modules through the alias' do
    expect(SFML::Audio::Sound).to equal(SF::Audio::Sound)
    expect(SFML::Graphics::Sprite).to equal(SF::Graphics::Sprite)
    expect(SFML::Window::Window).to equal(SF::Window::Window)
  end
end
