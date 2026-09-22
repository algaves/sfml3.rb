# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SF::Window::Vulkan do
  it 'reports availability as a boolean' do
    expect([true, false]).to include(SF::Window::Vulkan.available?)
  end
end
