# frozen_string_literal: true

require_relative '../spec_helper'

# Text needs a real font file and CI images do not all ship one, so every
# font-dependent example skips rather than fails when none is found.
RSpec.describe SFML::Text do
  def system_font_path
    candidates = [
      '/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf',
      '/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf',
      '/usr/share/fonts/google-noto/NotoSans-Regular.ttf',
      '/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf',
      '/Library/Fonts/Arial.ttf'
    ]

    candidates.find { |path| File.exist?(path) } ||
      Dir.glob('/usr/share/fonts/**/*.ttf').first
  end

  it 'round-trips non-ASCII strings' do
    path = system_font_path
    skip 'no system font available' unless path

    text = SFML::Text.new SFML::Font.from_file(path)

    # Goes through sfText_setUnicodeString: the plain char* entry point decodes
    # the bytes with the C locale and turns each non-ASCII byte into U+FFFFFFFF.
    ['hello', 'héllo', '日本語', 'emoji 🎮', ''].each do |string|
      text.string = string
      expect(text.string).to eq(string)
      expect(text.string.encoding).to eq(Encoding::UTF_8)
    end
  end
end
