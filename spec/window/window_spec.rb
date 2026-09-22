# frozen_string_literal: true

require_relative '../spec_helper'

# The Rubyesque (Matz-like) window names and the window Style/State flag
# constants, plus Window's own deprecated aliases. Hardware-free: `allocate`
# gives window objects whose Ruby-only methods can run without a display.
RSpec.describe SFML::Window do
  describe 'Rubyesque names' do
    it 'exposes the primary Rubyesque names' do
      %i[open? focused? visible? request_focus! poll_events!].each do |name|
        expect(SFML::WindowBase.instance_methods).to include(name)
      end

      %i[clear! display! render!].each do |name|
        expect(SFML::Window.instance_methods).to include(name)
      end

      expect(SFML::Window).to respond_to(:open)
      expect(SFML::RenderWindow).to respond_to(:open)
      expect(SFML::RenderWindow.instance_methods).to include(:render!)
    end

    it 'gives the Rubyesque names precedence over the deprecated ones' do
      expect(SFML::WindowBase.instance_method(:open?).owner).to eq(SFML::WindowBase)
      expect(SFML::Window.instance_method(:open?).owner).to eq(SFML::Window)
      expect(SFML::RenderWindow.instance_method(:open?).owner).to eq(SFML::Window)
      expect(SFML::Window.instance_method(:clear!).owner).to eq(SFML::Window)
    end

    it 'keeps the deprecated window names around' do
      %i[is_open? focus? visible?].each do |name|
        expect(SFML::WindowBase.instance_methods).to include(name)
      end

      %i[is_open? focus? request_focus clear display].each do |name|
        expect(SFML::Window.instance_methods).to include(name)
      end
    end

    it 'defaults visible to true without a display' do
      expect(SFML::WindowBase.allocate).to be_visible
    end

    it 'returns an Enumerator from poll_events! without a block' do
      expect(SFML::WindowBase.allocate.poll_events!).to be_a(Enumerator)
    end

    it 'declares the block forms of the event methods' do
      expect(SFML::WindowBase.instance_method(:open!).owner).to eq(SFML::WindowBase)
      expect(SFML::Window.instance_method(:poll_event!).owner).to eq(SFML::Window)
      expect(SFML::Window.instance_method(:wait_event!).owner).to eq(SFML::Window)

      expect(SFML::WindowBase.instance_method(:poll_event!).parameters).to include(%i[opt event])
      expect(SFML::WindowBase.instance_method(:wait_event!).parameters).to include(%i[opt event])
    end
  end

  describe 'Style and State' do
    it 'are integer flag namespaces' do
      expect([SFML::Style::NONE, SFML::Style::TITLEBAR, SFML::Style::RESIZE,
              SFML::Style::CLOSE, SFML::Style::DEFAULT]).to eq([0, 1, 2, 4, 7])
      expect(SFML::Style::TITLEBAR | SFML::Style::RESIZE | SFML::Style::CLOSE)
        .to eq(SFML::Style::DEFAULT)

      expect(SFML::State::WINDOWED).to eq(0)
      expect(SFML::State::FULLSCREEN).to eq(1)
    end
  end
end
