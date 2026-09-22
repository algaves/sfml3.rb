# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::Event do
  it 'reflects its type through the predicates' do
    event = SFML::Event.new

    expect(event).to be_closed
    expect(event).not_to be_key_pressed
    expect(event).not_to be_mouse_moved
    expect(event).to respond_to(:sensor_changed?)
  end

  it 'aliases code to the key code' do
    event = SFML::Event.new

    expect(event.code).to eq(event.key[:code])
  end
end
