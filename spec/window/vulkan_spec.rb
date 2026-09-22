# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::Vulkan do
  it 'reports availability as a boolean' do
    expect([true, false]).to include(SFML::Vulkan.available?)
  end
end
