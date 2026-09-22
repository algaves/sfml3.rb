# frozen_string_literal: true

require_relative '../spec_helper'

# The SoundRecorder tree, separate from the SoundSource tree: SoundRecorder is
# the base of SoundBufferRecorder. Structure only: recording needs an audio
# device.
RSpec.describe SFML::SoundRecorder do
  it 'keeps the recorder tree separate from the sound source tree' do
    expect(SFML::SoundRecorder).not_to be < SFML::SoundSource
    expect(SFML::SoundSource).not_to be < SFML::SoundRecorder
    expect(SFML::SoundBufferRecorder).not_to be < SFML::SoundSource
  end

  it 'reports recorder availability' do
    expect([true, false]).to include(SFML::SoundRecorder.available?)
    expect(SFML::SoundRecorder.available_devices).to be_a(Array)

    # A runner with no capture hardware reports no default device; CSFML
    # returns NULL and the binding maps that to nil rather than raising.
    expect([String, NilClass]).to include(SFML::SoundRecorder.default_device.class)
  end
end
