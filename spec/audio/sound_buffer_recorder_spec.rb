# frozen_string_literal: true

require_relative '../spec_helper'

RSpec.describe SF::Audio::SoundBufferRecorder do
  it 'makes SoundBufferRecorder a SoundRecorder' do
    expect(SF::Audio::SoundBufferRecorder).to be < SF::Audio::SoundRecorder
    expect(SF::Audio::SoundBufferRecorder.superclass).to eq(SF::Audio::SoundRecorder)
    expect(SF::Audio::SoundRecorder.superclass).to eq(Object)
  end

  it 'drops channel_map from SoundBufferRecorder' do
    expect(SF::Audio::SoundRecorder.instance_methods).to include(:channel_map)
    expect(SF::Audio::SoundBufferRecorder.instance_methods).not_to include(:channel_map)
  end

  it 'exposes the record block helper' do
    expect(SF::Audio::SoundBufferRecorder).to respond_to(:record!)
  end
end
