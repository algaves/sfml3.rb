# frozen_string_literal: true

# SFML::Vulkan: SFML's half of a Vulkan integration. `Vulkan.available?` says
# whether a loader is present, `graphics_required_instance_extensions` lists
# the instance extensions a Vulkan-capable window needs, and `Vulkan.function`
# resolves entry points.
#
# When a graphics-capable loader is present, the example tries to build a real
# VkInstance and hand it to `WindowBase#create_vulkan_surface`, which returns
# the VkSurfaceKHR SFML creates. There is no usable `vulkan` gem on RubyGems
# (the name resolves to an empty 0.0.0), so the FFI calls go through Fiddle,
# which is a stdlib gem outside Bundler; under `bundle exec` it is absent and
# the example just probes SFML's surface. Everything is guarded, so a machine
# with no Vulkan driver prints what is missing and exits cleanly.
#
# Run from the repository root with a display:
#   bundle exec ruby -Ilib examples/subsystems/vulkan.rb
# (headless: SFML_EXAMPLE_FRAMES=120 xvfb-run -a bundle exec ruby -Ilib ...)

require 'sfml'
require_relative '../support'
include SFML

HAVE_FIDDLE = begin
  require 'fiddle/import'
  true
rescue LoadError
  false
end

if HAVE_FIDDLE
  module VkStructs
    extend Fiddle::Importer

    ApplicationInfo = struct([
                               'unsigned int sType', 'void *pNext', 'char *pApplicationName',
                               'unsigned int applicationVersion', 'char *pEngineName',
                               'unsigned int engineVersion', 'unsigned int apiVersion'
                             ])

    InstanceCreateInfo = struct([
                                  'unsigned int sType', 'void *pNext', 'unsigned int flags',
                                  'void *pApplicationInfo', 'unsigned int enabledLayerCount',
                                  'void *ppEnabledLayerNames', 'unsigned int enabledExtensionCount',
                                  'void *ppEnabledExtensionNames'
                                ])
  end

  # Minimal vkCreateInstance / vkDestroyInstance calls. Returns the instance
  # handle (Integer), or raises with the VkResult on failure.
  module VulkanInstance
    VK_STRUCTURE_TYPE_APPLICATION_INFO = 0
    VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO = 1
    VK_API_VERSION_1_0 = 1 << 22
    VOIDP = Fiddle::SIZEOF_VOIDP
    PACK = VOIDP == 8 ? 'Q' : 'L'

    module_function

    def pointer_for(text)
      Fiddle::Pointer[text]
    end

    def create(extensions)
      application = VkStructs::ApplicationInfo.malloc
      application.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO
      application.pNext = 0
      application.pApplicationName = pointer_for('sfml3.rb example').to_i
      application.applicationVersion = 1
      application.pEngineName = pointer_for('sfml3.rb').to_i
      application.engineVersion = 1
      application.apiVersion = VK_API_VERSION_1_0

      names = extensions.map { |extension| pointer_for(extension) }
      block = Fiddle::Pointer.malloc(names.size * VOIDP)
      names.each_with_index { |pointer, index| block[index * VOIDP, VOIDP] = [pointer.to_i].pack(PACK) }

      info = VkStructs::InstanceCreateInfo.malloc
      info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO
      info.pNext = 0
      info.flags = 0
      info.pApplicationInfo = application.to_ptr.to_i
      info.enabledLayerCount = 0
      info.ppEnabledLayerNames = 0
      info.enabledExtensionCount = names.size
      info.ppEnabledExtensionNames = block.to_i

      out = Fiddle::Pointer.malloc(VOIDP)
      result = create_function.call(info.to_ptr, 0, out)
      raise "vkCreateInstance returned VkResult #{result}" unless result.zero?

      out[0, VOIDP].unpack1(PACK)
    end

    def destroy(instance)
      destroy_function.call(instance, 0)
    end

    def create_function
      address = SFML::Vulkan.function('vkCreateInstance')
      raise 'SFML::Vulkan.function(vkCreateInstance) returned 0' if address.zero?

      Fiddle::Function.new(address, [Fiddle::TYPE_VOIDP, Fiddle::TYPE_VOIDP, Fiddle::TYPE_VOIDP],
                           Fiddle::TYPE_INT)
    end

    def destroy_function
      address = SFML::Vulkan.function('vkDestroyInstance')
      raise 'SFML::Vulkan.function(vkDestroyInstance) returned 0' if address.zero?

      Fiddle::Function.new(address, [Fiddle::TYPE_VOIDP, Fiddle::TYPE_VOIDP], Fiddle::TYPE_VOID)
    end
  end
end

puts "Vulkan.available?(false)      -> #{Vulkan.available?(false)}"
puts "Vulkan.available?(true)       -> #{Vulkan.available?(true)}"

extensions = Vulkan.available?(false) ? Vulkan.graphics_required_instance_extensions : []
puts "required graphics extensions  -> #{extensions.inspect}"
%w[vkGetInstanceProcAddr vkCreateInstance vkDestroyInstance].each do |name|
  puts format('Vulkan.function(%<name>-20s) -> 0x%<address>x', name: name, address: Vulkan.function(name))
end

unless Vulkan.available?(true)
  puts "\nNo graphics-capable Vulkan here; skipping the window/surface step."
  exit 0
end

window = Window.new(VideoMode.new(640, 480, 32), 'SFML Vulkan')
instance = nil

begin
  if HAVE_FIDDLE
    instance = VulkanInstance.create(extensions)
    surface = window.create_vulkan_surface(instance)
    puts "\ncreated VkInstance   0x#{instance.to_s(16)}"
    puts "create_vulkan_surface -> VkSurfaceKHR 0x#{surface.to_s(16)}"
  else
    puts "\nFiddle is unavailable under Bundler; showing SFML's surface only."
    puts 'Run without `bundle exec` (or add fiddle to the Gemfile) to build a VkInstance.'
  end
rescue StandardError => e
  puts "\nVulkan window setup failed: #{e.class}: #{e.message}"
end

event = Event.new
frame = 0

loop do
  while window.poll_event!(event)
    case event.type
    when 'closed'
      window.close!
    when 'key-pressed'
      window.close! if event.key[:code] == :escape
    end
  end

  window.clear([18, 20, 30, 255])
  window.draw(ExampleSupport.text(
                "SFML::Vulkan probe complete -- see the console for details.\n" \
                "#{extensions.size} required instance extension(s). Escape quits.",
                size: 16
              ))
  window.display

  frame += 1
  break if ExampleSupport.auto_close?(frame)
end

VulkanInstance.destroy(instance) if HAVE_FIDDLE && instance && !instance.zero?
