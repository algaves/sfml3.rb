# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SFML::SoundBufferRecorder do
  it 'makes SoundBufferRecorder a SoundRecorder' do
    expect(SFML::SoundBufferRecorder).to be < SFML::SoundRecorder
    expect(SFML::SoundBufferRecorder.superclass).to eq(SFML::SoundRecorder)
    expect(SFML::SoundRecorder.superclass).to eq(Object)
  end

  it 'drops channel_map from SoundBufferRecorder' do
    expect(SFML::SoundRecorder.instance_methods).to include(:channel_map)
    expect(SFML::SoundBufferRecorder.instance_methods).not_to include(:channel_map)
  end

  it 'exposes the record block helper' do
    expect(SFML::SoundBufferRecorder).to respond_to(:record!)
  end
end
