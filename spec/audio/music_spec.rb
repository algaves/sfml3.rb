# frozen_string_literal: true

require_relative '../spec_helper'
require 'tmpdir'
require 'fileutils'

RSpec.describe SF::Audio::Music do
  it 'opens music from a file' do
    path = File.join(Dir.tmpdir, "sfml3_rb_music_#{Process.pid}.wav")

    begin
      SF::Audio::SoundBuffer.from_samples(Array.new(200) { |i| (i % 100) - 50 }, 2, 44_100).save_to_file(path)
      music = SF::Audio::Music.from_file(path)

      expect(music.channel_count).to eq(2)
      expect(music.sample_rate).to eq(44_100)
      expect(music.duration.as_seconds).to be > 0
      expect(music.status).to eq(:stopped)
    ensure
      FileUtils.rm_f(path)
    end
  end
end
