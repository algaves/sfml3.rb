# frozen_string_literal: true

require_relative '../spec_helper'

# Touch/Sensor/Joystick/Keyboard share one Rubyesque naming surface and are
# device-free, so their predicate spellings are asserted here together.
RSpec.describe 'Rubyesque input devices' do
  it 'exposes the Rubyesque names' do
    expect(SFML::Sensor).to respond_to(:enable!)
    expect(SFML::Sensor).to respond_to(:disable!)
    expect(SFML::Joystick).to respond_to(:axis?)
    expect(SFML::Keyboard).to respond_to(:key_pressed?)
    expect(SFML::Sensor).to respond_to(:available?)
    expect(SFML::Touch).to respond_to(:down?)
  end

  it 'has Touch.position take the relative_to keyword' do
    parameters = SFML::Touch.method(:position).parameters

    expect(parameters).to include(%i[key relative_to])
    expect(parameters).to include(%i[opt window])
  end
end
