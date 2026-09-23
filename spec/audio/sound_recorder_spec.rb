# frozen_string_literal: true

require_relative '../spec_helper'

# The SoundRecorder tree, separate from the SoundSource tree: SoundRecorder is
# the base of SoundBufferRecorder. Structure only: recording needs an audio
# device.
RSpec.describe SF::Audio::SoundRecorder do
  it 'keeps the recorder tree separate from the sound source tree' do
    expect(SF::Audio::SoundRecorder).not_to be < SF::Audio::SoundSource
    expect(SF::Audio::SoundSource).not_to be < SF::Audio::SoundRecorder
    expect(SF::Audio::SoundBufferRecorder).not_to be < SF::Audio::SoundSource
  end

  it 'reports recorder availability' do
    expect([true, false]).to include(SF::Audio::SoundRecorder.available?)
    expect(SF::Audio::SoundRecorder.available_devices).to be_a(Array)

    # A runner with no capture hardware reports no default device; CSFML
    # returns NULL and the binding maps that to nil rather than raising.
    expect([String, NilClass]).to include(SF::Audio::SoundRecorder.default_device.class)
  end
end
