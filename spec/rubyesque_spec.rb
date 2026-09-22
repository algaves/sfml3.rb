# frozen_string_literal: true

require_relative 'spec_helper'

# The Rubyesque (Matz-like) layer's per-class tests live alongside their
# classes in the category folders; only the cross-cutting deprecated aliases
# that span the layer stay here.
RSpec.describe 'Rubyesque deprecated aliases' do
  it 'warns and works through the deprecated SFML.sleep spelling' do
    expect { SFML.sleep(0) }.to output(/SFML\.sleep is deprecated/).to_stderr
  end

  it 'warns and works through the deprecated Clipboard.string= spelling' do
    expect { SFML::Clipboard.string = 'old spelling' }.to output(/Clipboard\.string=/i).to_stderr
    expect(SFML::Clipboard.content).to eq('old spelling')
  end
end
