# frozen_string_literal: true

# SF::Window::Vulkan: SFML's half of a Vulkan integration. `Vulkan.available?`
# says whether a loader is present, `graphics_required_instance_extensions`
# lists the instance extensions a Vulkan-capable window needs, and
# `Vulkan.function` resolves entry points.
#
# Building a real VkInstance to hand to `WindowBase#create_vulkan_surface` needs
# a Vulkan loader and the struct marshalling to call vkCreateInstance, which is
# left out here to keep the example small; SFML's surface creation is exercised
# by passing in a handle you obtain yourself (e.g. via a Fiddle import). A
# machine with no Vulkan driver prints what is missing and exits cleanly.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/vulkan/vulkan.rb
# (headless: prefix `xvfb-run -a`).

require 'sfml'
include SF::Window
include SF::Graphics
include SF::System

# --- self-contained helpers -------------------------------------------------
ASSETS = File.expand_path('assets', __dir__)

FONT = Font.from_file(File.join(ASSETS, 'LiberationSans-Regular.ttf'))
def text(string, size: 18, position: [12, 8], color: [235, 235, 245, 255])
  label = Text.new(FONT, string, size)
  label.fill_color = color
  label.position = position
  label
end

puts "Vulkan.available?(false)      -> #{Vulkan.available?(false)}"
puts "Vulkan.available?(true)       -> #{Vulkan.available?(true)}"

extensions = Vulkan.available?(false) ? Vulkan.graphics_required_instance_extensions : []
puts "required graphics extensions  -> #{extensions.inspect}"
%w[vkGetInstanceProcAddr vkCreateInstance vkDestroyInstance].each do |name|
  puts format('Vulkan.function(%<name>-20s) -> 0x%<address>x', name: name, address: Vulkan.function(name))
end

unless Vulkan.available?(true)
  puts "\nNo graphics-capable Vulkan here; skipping the window step."
  exit 0
end

window = Window.new(VideoMode.new(640, 480, 32), 'SFML Vulkan')

while window.open?
  window.poll_events! do |event|
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      window.close! if event.code == :escape
    end
  end

  window.clear!([18, 20, 30, 255])
  window.draw(text(
                "SF::Window::Vulkan probe complete -- see the console for details.\n" \
                "#{extensions.size} required instance extension(s). Escape quits.",
                size: 16
              ))
  window.display!

end
