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

    expect(output).not_to include('undefining the allocator')
  end
end
