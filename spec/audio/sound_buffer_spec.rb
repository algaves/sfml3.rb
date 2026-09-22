# frozen_string_literal: true

require_relative '../spec_helper'
require 'tmpdir'
require 'fileutils'

RSpec.describe SFML::SoundBuffer do
  it 'builds a buffer from samples' do
    buffer = SFML::SoundBuffer.from_samples([0, 100, -100, 0], 1, 44_100)

    expect(buffer.sample_count).to eq(4)
    expect(buffer.sample_rate).to eq(44_100)
    expect(buffer.channel_count).to eq(1)
    expect(buffer.samples).to eq([0, 100, -100, 0])
    expect(buffer.channel_map).to eq([:mono])
    expect(buffer.duration.as_seconds).to be_within(0.0001).of(4.0 / 44_100)
  end

  it 'builds a buffer from a packed string' do
    packed = [10, -10, 20, -20].pack('s<*')
    buffer = SFML::SoundBuffer.from_samples(packed, 1, 22_050)

    expect(buffer.sample_count).to eq(4)
    expect(buffer.samples).to eq([10, -10, 20, -20])
  end

  it 'copies and saves a buffer' do
    buffer = SFML::SoundBuffer.from_samples([1, 2, 3, 4], 1, 8000)
    copy = buffer.copy
    expect(copy.sample_count).to eq(buffer.sample_count)

    path = File.join(Dir.tmpdir, "sfml3_rb_test_#{Process.pid}.wav")
    begin
      expect(buffer.save_to_file(path)).to be(true)
      reloaded = SFML::SoundBuffer.from_file(path)
      expect(reloaded.sample_count).to eq(buffer.sample_count)
      expect(reloaded.sample_rate).to eq(buffer.sample_rate)
    ensure
      FileUtils.rm_f(path)
    end
  end

  it 'loads a buffer from a stream' do
    path = File.join(Dir.tmpdir, "sfml3_rb_stream_#{Process.pid}.wav")

    begin
      SFML::SoundBuffer.from_samples([1, 2, 3, 4, 5, 6], 1, 8000).save_to_file(path)
      buffer = SFML::SoundBuffer.from_stream(File.open(path, 'rb'))
      expect(buffer.sample_count).to eq(6)
    ensure
      FileUtils.rm_f(path)
    end
  end
end
