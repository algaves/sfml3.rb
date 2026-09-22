# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SF::Window::ContextSettings do
  it 'constructs, queries and mutates' do
    settings = SF::Window::ContextSettings.new(24, 8, 4, 3, 3, [:core], false)
    expect(settings.depth_bits).to eq(24)
    expect(settings.antialiasing_level).to eq(4)
    expect(settings.attribute_flags).to eq([:core])
    expect(settings).not_to be_srgb_capable

    settings.attribute_flags = :debug
    expect(settings.attribute_flags).to eq([:debug])
  end
end
