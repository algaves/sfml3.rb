# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::Sleep do
  it 'accepts a Time or a number' do
    expect(SFML.sleep(0)).to be_within(0.001).of(0.0)
    expect(SFML.sleep(SFML::Time.zero)).to be_a(SFML::Time)
  end

  it 'delegates the Sleep module to the top level' do
    expect(SFML::Sleep.sleep!(0)).to be_within(0.001).of(0.0)
  end
end
