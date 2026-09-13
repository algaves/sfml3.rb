require 'digest'
require 'etc'
require 'fileutils'
require 'open-uri'
require 'rbconfig'

# Downloads, verifies and builds SFML 3 and CSFML 3 from source into
# ports/<host>, static and position-independent, so the extension links them
# directly and the resulting .so has no libsfml/libcsfml runtime dependency.
#
# This lives under ext/ rather than rakelib/ because the gemspec ships
# ext/**/*.rb: it has to be inside the gem for `gem install` to build its own
# dependencies. For the same reason it uses no Rake helpers -- Rake is not
# guaranteed to be loaded during an extension build.
#
# Two entry points call it: `rake ports` (development) and ext/extconf.rb
# (install time).
module Ports
  ROOT = File.expand_path('../ports', __dir__)
  ARCHIVES = File.join(ROOT, 'archives')
  BUILD = File.join(ROOT, 'build')
  PREFIX = File.join(ROOT, RbConfig::CONFIG['host'])

  # CMAKE_INSTALL_LIBDIR is pinned because GNUInstallDirs otherwise picks lib64
  # on Fedora/RHEL and lib on Debian, which would split SFML and CSFML across
  # two directories.
  COMMON_FLAGS = %W[
    -DCMAKE_INSTALL_PREFIX=#{PREFIX}
    -DCMAKE_PREFIX_PATH=#{PREFIX}
    -DCMAKE_BUILD_TYPE=Release
    -DBUILD_SHARED_LIBS=OFF
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    -DCMAKE_INSTALL_LIBDIR=lib
  ].freeze

  # Order matters: CSFML does find_package(SFML 3 ... REQUIRED) and does not
  # fetch SFML itself, so SFML must already be installed into PREFIX.
  # Audio and network are off because the binding wraps neither, which also
  # drops the FLAC/Ogg/Vorbis and mbedtls dependency families.
  RECIPES = [
    {
      name: 'SFML',
      version: '3.0.2',
      sha256: '0034e05f95509e5d3fb81b1625713e06da7b068f210288ce3fd67106f8f46995',
      flags: %w[
        -DSFML_BUILD_AUDIO=OFF
        -DSFML_BUILD_NETWORK=OFF
        -DSFML_BUILD_EXAMPLES=OFF
        -DSFML_BUILD_DOC=OFF
        -DSFML_BUILD_TEST_SUITE=OFF
      ]
    },
    {
      name: 'CSFML',
      version: '3.0.0',
      sha256: '903cd4a782fb0b233f732dc5b37861b552998e93ae8f268c40bd4ce50b2e88ca',
      flags: %w[
        -DCSFML_BUILD_AUDIO=OFF
        -DCSFML_BUILD_NETWORK=OFF
        -DCSFML_BUILD_EXAMPLES=OFF
        -DCSFML_LINK_SFML_STATICALLY=ON
      ]
    }
  ].freeze

  TOOLCHAIN_HINT = <<~HINT.freeze
    Building SFML from source needs CMake >= 3.22, a C++17 compiler, and the
    X11/udev/OpenGL development headers. SFML links those from the system and
    they cannot be bundled.

      Fedora/RHEL    sudo dnf install cmake gcc-c++ freetype-devel libX11-devel \\
                       libXrandr-devel libXcursor-devel libXi-devel systemd-devel libglvnd-devel

      Debian/Ubuntu  sudo apt-get install cmake build-essential libfreetype-dev libx11-dev \\
                       libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev

    If you already have CSFML 3 installed system-wide, skip this build with:

      gem install sfml3-rb -- --enable-system-libraries
  HINT

  module_function

  def built?
    Dir.exist?(File.join(PREFIX, 'include', 'CSFML'))
  end

  def build!
    RECIPES.each do |recipe|
      fetch(recipe)
      compile(recipe)
    end
  end

  def tarball(recipe)
    File.join(ARCHIVES, "#{recipe[:name]}-#{recipe[:version]}.tar.gz")
  end

  def source_dir(recipe)
    File.join(BUILD, "#{recipe[:name]}-#{recipe[:version]}")
  end

  def fetch(recipe)
    path = tarball(recipe)
    download(recipe, path) unless File.exist?(path)

    actual = Digest::SHA256.file(path).hexdigest
    return if actual == recipe[:sha256]

    # Never build unverified source: remove it so a retry re-downloads.
    File.delete(path)
    raise "#{File.basename(path)} failed checksum verification\n" \
          "expected #{recipe[:sha256]}\nactual   #{actual}"
  end

  def download(recipe, path)
    url = "https://github.com/SFML/#{recipe[:name]}/archive/refs/tags/#{recipe[:version]}.tar.gz"
    puts "Downloading #{url}"

    FileUtils.mkdir_p(ARCHIVES)
    partial = "#{path}.part"

    begin
      # URI.parse(...).open rather than URI.open: the latter routes through
      # Kernel#open semantics, where a "|command" string would be executed.
      URI.parse(url).open { |remote| IO.copy_stream(remote, partial) }
    rescue StandardError => e
      FileUtils.rm_f(partial)
      raise "Could not download #{url}: #{e.message}\n\n" \
            "`gem install` needs network access to build SFML from source.\n" \
            "#{TOOLCHAIN_HINT}"
    end

    # Only becomes the real tarball once complete, so an interrupted download
    # cannot be mistaken for a cached one on the next run.
    FileUtils.mv(partial, path)
  end

  def compile(recipe)
    source = source_dir(recipe)
    build = "#{source}-build"

    unless Dir.exist?(source)
      FileUtils.mkdir_p(BUILD)
      run('tar', 'xzf', tarball(recipe), '-C', BUILD)
    end

    puts "Building #{recipe[:name]} #{recipe[:version]} into #{PREFIX}"
    run('cmake', '-S', source, '-B', build, *COMMON_FLAGS, *recipe[:flags])
    run('cmake', '--build', build, '--parallel', Etc.nprocessors.to_s)
    run('cmake', '--install', build)
  end

  def run(*command)
    return if system(*command)

    raise "Command failed: #{command.join(' ')}\n\n#{TOOLCHAIN_HINT}"
  end
end
