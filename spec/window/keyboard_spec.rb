# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::Keyboard do
  it 'maps between keys and scancodes' do
    expect(SFML::Keyboard.delocalize(:a)).to eq(0)
    expect(SFML::Keyboard.localize(4)).to eq(:e)
    expect(SFML::Keyboard.delocalize('a')).to eq(0)
  end

  it 'rejects unknown keys' do
    expect { SFML::Keyboard.delocalize(:not_a_key) }.to raise_error(ArgumentError)
  end
end
