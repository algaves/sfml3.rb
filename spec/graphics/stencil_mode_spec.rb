# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::StencilMode do
  it 'constructs a stencil mode' do
    mode = SFML::StencilMode.new(:equal, :replace, 3, 0xFF, true)
    expect(mode.comparison).to eq(:equal)
    expect(mode.update_operation).to eq(:replace)
    expect(mode.reference).to eq(3)
    expect(mode.mask).to eq(0xFF)
    expect(mode.stencil_only).to be(true)
  end
end
