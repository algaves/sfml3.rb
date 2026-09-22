# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SF::Window::Event do
  it 'reflects its type through the predicates' do
    event = SF::Window::Event.new

    expect(event).to be_closed
    expect(event).not_to be_key_pressed
    expect(event).not_to be_mouse_moved
    expect(event).to respond_to(:sensor_changed?)
  end

  it 'aliases code to the key code' do
    event = SF::Window::Event.new
    key = event.key

    expect(event.code).to eq(key[:code])
    expect(key[:code]).to be_a(Symbol)
    expect(key[:scancode]).to be_a(String)
    expect(key.values_at(:alt, :control, :shift, :system)).to all(be(false))

    100.times { expect(event.key).to eq(key) }
  end
end
