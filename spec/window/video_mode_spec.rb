# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SF::Window::VideoMode do
  it 'constructs with integer fields' do
    vm = SF::Window::VideoMode.new 800, 600, 32
    expect(vm.width).to be_a(Integer)
    expect(vm.height).to be_a(Integer)
    expect(vm.bits).to be_a(Integer)
  end

  it 'builds from bracket constructors' do
    mode = SF::Window::VideoMode[640, 480, 32]

    expect(mode).to be_a(SF::Window::VideoMode)
    expect([mode.width, mode.height]).to eq([640, 480])
    expect(mode.bits).to eq(32)
    expect(SF::Window::VideoMode[800, 600].bits).to eq(32)
  end
end
