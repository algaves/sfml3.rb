# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SF::Graphics::BlendMode do
  it 'exposes the predefined modes' do
    expect(SF::Graphics::BlendMode::MULTIPLY.color_src_factor).to eq(:dst_color)
    expect(SF::Graphics::BlendMode::MULTIPLY.color_equation).to eq(:add)
    expect(SF::Graphics::BlendMode::ALPHA.alpha_src_factor).to eq(:one)
  end

  it 'constructs a custom blend mode' do
    mode = SF::Graphics::BlendMode.new(:src_alpha, :one_minus_src_alpha, :add, :one, :zero, :add)
    expect(mode.color_src_factor).to eq(:src_alpha)
    expect(mode.alpha_dst_factor).to eq(:zero)
  end
end
