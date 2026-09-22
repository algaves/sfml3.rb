# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::VideoMode do
  it 'constructs with integer fields' do
    vm = SFML::VideoMode.new 800, 600, 32
    expect(vm.width).to be_a(Integer)
    expect(vm.height).to be_a(Integer)
    expect(vm.bits).to be_a(Integer)
  end

  it 'builds from bracket constructors' do
    mode = SFML::VideoMode[640, 480, 32]

    expect(mode).to be_a(SFML::VideoMode)
    expect([mode.width, mode.height]).to eq([640, 480])
    expect(mode.bits).to eq(32)
    expect(SFML::VideoMode[800, 600].bits).to eq(32)
  end
end
