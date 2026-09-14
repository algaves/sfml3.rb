require 'digest'
require 'etc'
require 'fileutils'
require 'open-uri'
require 'rbconfig'

# Downloads, verifies and builds FreeType, SFML 3 and CSFML 3 from source into
# ports/<target>, static and position-independent, so the extension links them
# directly and the resulting .so has no libsfml/libcsfml runtime dependency.
#
# This lives under ext/ rather than rakelib/ because the gemspec ships
# ext/**/*.rb: it has to be inside the gem for `gem install` to build its own
# dependencies. For the same reason it uses no Rake helpers -- Rake is not
# guaranteed to be loaded during an extension build.
#
# Three entry points call it: `rake ports` (development), ext/extconf.rb
# (source-gem install time), and `rake gem:native` (cross-compilation inside
# rake-compiler-dock, which exports SFML_TARGET).
module Ports
  ROOT = File.expand_path('../ports', __dir__)
  ARCHIVES = File.join(ROOT, 'archives')

  # Cross targets, keyed by the RubyGems platform name that rake-compiler and
  # rake-compiler-dock use for them. `triple` is the GNU host triple whose
  # toolchain the matching rake-compiler-dock image installs as <triple>-gcc.
  #
  # `multiarch` is the Debian multiarch directory name, which is only sometimes
  # the same string as the triple: i686 compiles with i686-linux-gnu-gcc but its
  # libraries live in /usr/lib/i386-linux-gnu. Only set it where they differ.
  TARGETS = {
    'x86_64-linux-gnu' => { triple: 'x86_64-linux-gnu', os: :linux, cpu: 'x86_64' },
    'x86-linux-gnu' => { triple: 'i686-linux-gnu', multiarch: 'i386-linux-gnu',
                         os: :linux, cpu: 'i686' },
    'aarch64-linux-gnu' => { triple: 'aarch64-linux-gnu', os: :linux, cpu: 'aarch64' },
    'x86_64-linux-musl' => { triple: 'x86_64-unknown-linux-musl', os: :linux, cpu: 'x86_64' },
    'x86-linux-musl' => { triple: 'i686-unknown-linux-musl', os: :linux, cpu: 'i686' },
    'x64-mingw-ucrt' => { triple: 'x86_64-w64-mingw32', os: :windows, cpu: 'x86_64' },
    'x86-mingw32' => { triple: 'i686-w64-mingw32', os: :windows, cpu: 'i686' },
    'x86_64-darwin' => { triple: 'x86_64-apple-darwin', os: :darwin, cpu: 'x86_64' },
    'arm64-darwin' => { triple: 'aarch64-apple-darwin', os: :darwin, cpu: 'arm64' }
  }.freeze

  CMAKE_SYSTEM = { linux: 'Linux', windows: 'Windows', darwin: 'Darwin' }.freeze

  # osxcross ships clang wrappers rather than gcc ones.
  COMPILERS = {
    darwin: %w[clang clang++],
    linux: %w[gcc g++],
    windows: %w[gcc g++]
  }.freeze

  # What the extension links on top of the static SFML/CSFML/FreeType/codec
  # archives. SFML resolves GL entry points through its own loader, so on Linux
  # libGL is only needed for the handful of symbols SFML references directly.
  # Network links ws2_32 on Windows; audio needs no OpenAL, because SFML 3 uses
  # the vendored miniaudio backend and resolves ALSA/Pulse at run time via dl.
  SYSTEM_LIBS = {
    linux: %w[GL X11 Xrandr Xcursor Xi udev pthread dl rt m],
    windows: %w[opengl32 winmm gdi32 user32 advapi32 ole32 ws2_32],
    darwin: %w[]
  }.freeze

  # Clang wants -framework, not -l, and SFML's macOS backend is Objective-C++.
  # Audio reaches CoreAudio/CoreFoundation from miniaudio, and the network
  # stack needs nothing beyond the system libc.
  FRAMEWORKS = {
    darwin: %w[Cocoa OpenGL IOKit Carbon CoreFoundation CoreAudio AudioToolbox],
    linux: [], windows: []
  }.freeze

  # Order matters. The Ogg/Vorbis/FLAC codecs and FreeType must be installed
  # before SFML configures, because SFML's find_package(Vorbis)/find_package(FLAC)
  # run with SFML_USE_SYSTEM_DEPS=ON and must find our static archives rather
  # than the build host's shared ones. CSFML then does find_package(SFML 3 ...)
  # and does not fetch SFML itself.
  #
  # FreeType is built here rather than left to SFML because SFML's own
  # FetchContent path clones it from git (no checksum, needs git at configure
  # time) and then does not install the resulting archive -- which leaves
  # libsfml-graphics-s.a with an undefined FT_Init_FreeType that only stays
  # latent while nothing binds sf::Font. Pinning it as a port fixes both. The
  # same reasoning applies to the codecs: vendoring them keeps the binary gems
  # free of libvorbis/libFLAC/libogg runtime dependencies and makes the audio
  # module buildable on the mingw/darwin/musl images, which ship no codec
  # development files at all.
  RECIPES = [
    {
      name: 'libogg',
      version: '1.3.5',
      sha256: '0eb4b4b9420a0f51db142ba3f9c64b333f826532dc0f48c6410ae51f4799b664',
      url: 'https://downloads.xiph.org/releases/ogg/libogg-%<version>s.tar.gz',
      flags: %w[
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
        -DBUILD_SHARED_LIBS=OFF
        -DBUILD_TESTING=OFF
        -DINSTALL_DOCS=OFF
        -DINSTALL_CMAKE_PACKAGE_MODULE=ON
        -DINSTALL_PKG_CONFIG_MODULE=OFF
      ]
    },
    {
      name: 'libvorbis',
      version: '1.3.7',
      sha256: '0e982409a9c3fc82ee06e08205b1355e5c6aa4c36bca58146ef399621b0ce5ab',
      url: 'https://downloads.xiph.org/releases/vorbis/libvorbis-%<version>s.tar.gz',
      flags: %w[
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
        -DBUILD_SHARED_LIBS=OFF
        -DBUILD_TESTING=OFF
        -DINSTALL_CMAKE_PACKAGE_MODULE=ON
      ]
    },
    {
      name: 'flac',
      version: '1.4.3',
      sha256: '6c58e69cd22348f441b861092b825e591d0b822e106de6eb0ee4d05d27205b70',
      url: 'https://downloads.xiph.org/releases/flac/flac-%<version>s.tar.xz',
      ext: 'tar.xz',
      flags: %w[
        -DBUILD_SHARED_LIBS=OFF
        -DBUILD_CXXLIBS=OFF
        -DBUILD_PROGRAMS=OFF
        -DBUILD_EXAMPLES=OFF
        -DBUILD_TESTING=OFF
        -DBUILD_DOCS=OFF
        -DWITH_OGG=OFF
        -DWITH_FORTIFY_SOURCE=OFF
        -DWITH_STACK_PROTECTOR=OFF
        -DINSTALL_MANPAGES=OFF
        -DINSTALL_CMAKE_CONFIG_MODULE=ON
        -DINSTALL_PKGCONFIG_MODULES=ON
      ]
    },
    {
      name: 'freetype',
      version: '2.13.2',
      sha256: '1ac27e16c134a7f2ccea177faba19801131116fd682efc1f5737037c5db224b5',
      url: 'https://download.savannah.gnu.org/releases/freetype/freetype-%<version>s.tar.gz',
      # FreeType 2.13.2 still declares cmake_minimum_required(VERSION 3.0),
      # which CMake 4 refuses outright. Older CMake ignores the variable.
      flags: %w[
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
        -DFT_DISABLE_ZLIB=ON
        -DFT_DISABLE_BZIP2=ON
        -DFT_DISABLE_PNG=ON
        -DFT_DISABLE_HARFBUZZ=ON
        -DFT_DISABLE_BROTLI=ON
      ]
    },
    {
      name: 'SFML',
      version: '3.0.2',
      sha256: '0034e05f95509e5d3fb81b1625713e06da7b068f210288ce3fd67106f8f46995',
      flags: %w[
        -DSFML_USE_SYSTEM_DEPS=ON
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
        -DCSFML_BUILD_EXAMPLES=OFF
        -DCSFML_LINK_SFML_STATICALLY=ON
      ]
    }
  ].freeze

  TOOLCHAIN_HINT = <<~HINT.freeze
    Building SFML from source needs CMake >= 3.22, a C++17 compiler, and the
    X11/udev/OpenGL development headers. SFML links those from the system and
    they cannot be bundled.

      Fedora/RHEL    sudo dnf install cmake gcc-c++ libX11-devel \\
                       libXrandr-devel libXcursor-devel libXi-devel systemd-devel libglvnd-devel

      Debian/Ubuntu  sudo apt-get install cmake build-essential libx11-dev \\
                       libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev

    If you already have CSFML 3 installed system-wide, skip this build with:

      gem install sfml3-rb -- --enable-system-libraries

    A precompiled binary gem may also be available for your platform; upgrading
    RubyGems (gem update --system) lets it be selected automatically.
  HINT

  module_function

  # The RubyGems platform being built for. rake-compiler-dock invocations set
  # this explicitly; a native build falls back to the host triple, which is not
  # a TARGETS key and so uses the native toolchain and native system headers.
  def target
    ENV.fetch('SFML_TARGET', nil) || RbConfig::CONFIG['host']
  end

  def cross?
    TARGETS.key?(target)
  end

  def spec
    TARGETS.fetch(target)
  end

  # Each target gets its own prefix so a cross build never reuses the host's
  # archives -- they are the same file names with an incompatible ABI.
  def prefix
    File.join(ROOT, target)
  end

  def build_root
    File.join(ROOT, 'build', target)
  end

  def os
    return spec[:os] if cross?

    case RbConfig::CONFIG['host_os']
    when /darwin/ then :darwin
    when /mingw|mswin|cygwin/ then :windows
    else :linux
    end
  end

  def libs
    SYSTEM_LIBS.fetch(os)
  end

  def frameworks
    FRAMEWORKS.fetch(os)
  end

  # On Windows the CSFML and SFML headers declare every entry point
  # __declspec(dllimport) unless told the build is static, which leaves the
  # link hunting for __imp_-prefixed symbols that a static archive never has.
  # Applies to any Windows build against the vendored ports, cross or native.
  def defines
    return [] unless os == :windows

    %w[-DCSFML_STATIC -DSFML_STATIC]
  end

  # SFML is C++, but mkmf links the extension with `gcc -shared` because every
  # source here is C -- so libstdc++ has to be named explicitly, and
  # -static-libstdc++ does nothing, since it only redirects the -lstdc++ the
  # driver would have added itself.
  #
  # For a binary gem that matters: a Windows user has no libstdc++-6.dll, and
  # a musl one may have no libstdc++ at all. -Bstatic/-Bdynamic pins just this
  # one library to its archive and is understood by both GNU ld and mingw's.
  # macOS resolves C++ through the system libc++ and needs none of it.
  def cxx_runtime
    return [] if os == :darwin
    return ['-lstdc++'] unless cross?

    ['-Wl,-Bstatic', '-lstdc++', '-Wl,-Bdynamic']
  end

  def built?
    Dir.exist?(File.join(prefix, 'include', 'CSFML'))
  end

  def build!
    RECIPES.each do |recipe|
      fetch(recipe)
      compile(recipe)
    end
  end

  # The local archive keeps a fixed <name>-<version>.<ext> name regardless of
  # what the upstream release calls it, so recipes can point at libogg-1.3.5.tar.gz
  # or flac-1.4.3.tar.xz without the rest of the code caring.
  def tarball(recipe)
    File.join(ARCHIVES, "#{recipe[:name]}-#{recipe[:version]}.#{recipe[:ext] || 'tar.gz'}")
  end

  def source_dir(recipe)
    File.join(build_root, "#{recipe[:name]}-#{recipe[:version]}")
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

  def url_for(recipe)
    template = recipe[:url] ||
               "https://github.com/SFML/#{recipe[:name]}/archive/refs/tags/%<version>s.tar.gz"
    format(template, version: recipe[:version])
  end

  def download(recipe, path)
    url = url_for(recipe)
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

  # CMAKE_INSTALL_LIBDIR is pinned because GNUInstallDirs otherwise picks lib64
  # on Fedora/RHEL and lib on Debian, which would split the ports across two
  # directories that extconf would then both have to know about.
  def common_flags
    flags = %W[
      -DCMAKE_INSTALL_PREFIX=#{prefix}
      -DCMAKE_PREFIX_PATH=#{prefix}
      -DCMAKE_BUILD_TYPE=Release
      -DBUILD_SHARED_LIBS=OFF
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
      -DCMAKE_INSTALL_LIBDIR=lib
    ]
    flags << "-DCMAKE_TOOLCHAIN_FILE=#{toolchain_file}" if cross?
    flags
  end

  # The musl toolchain rake-compiler-dock builds with musl-cross-make installs
  # its sysroot here, and build/provision.sh unpacks Alpine's X11 development
  # files into it.
  def musl_sysroot
    "/usr/#{spec[:triple]}"
  end

  # osxcross keeps the macOS SDK under its target directory; the exact version
  # tracks the rake-compiler-dock image, so find it rather than pin it.
  def osx_sysroot
    ENV.fetch('SFML_OSX_SYSROOT', nil) ||
      Dir.glob('/opt/osxcross/target/SDK/MacOSX*.sdk').max ||
      raise("No macOS SDK found under /opt/osxcross/target/SDK.\n#{TOOLCHAIN_HINT}")
  end

  # Written rather than shipped because the prefix and triple are only known at
  # build time, and CMake needs a real file path for CMAKE_TOOLCHAIN_FILE.
  def toolchain_file
    path = File.join(build_root, 'toolchain.cmake')
    return path if File.exist?(path)

    FileUtils.mkdir_p(build_root)
    File.write(path, toolchain_source)
    path
  end

  def toolchain_source
    cc, cxx = COMPILERS.fetch(spec[:os])
    roots = [prefix]
    lines = [
      "set(CMAKE_SYSTEM_NAME #{CMAKE_SYSTEM.fetch(spec[:os])})",
      "set(CMAKE_SYSTEM_PROCESSOR #{spec[:cpu]})",
      "set(CMAKE_C_COMPILER #{spec[:triple]}-#{cc})",
      "set(CMAKE_CXX_COMPILER #{spec[:triple]}-#{cxx})"
    ]

    case spec[:os]
    when :windows
      lines << "set(CMAKE_RC_COMPILER #{spec[:triple]}-windres)"
    when :darwin
      # SFML's macOS backend is Objective-C++, so the SDK has to be visible to
      # the frameworks lookup as well as to the compiler.
      lines << "set(CMAKE_OSX_SYSROOT #{osx_sysroot})"
      lines << "set(CMAKE_OSX_ARCHITECTURES #{spec[:cpu]})"
      roots << osx_sysroot
    when :linux
      if spec[:triple].include?('musl')
        lines << "set(CMAKE_SYSROOT #{musl_sysroot})"
        roots << musl_sysroot << "#{musl_sysroot}/usr"
      else
        # Debian multiarch: without this CMake searches /usr/lib, finds the
        # container's own amd64 libraries and hands them to an aarch64 linker.
        lines << "set(CMAKE_LIBRARY_ARCHITECTURE #{spec[:multiarch] || spec[:triple]})"
      end
    end

    <<~CMAKE
      #{lines.join("\n")}

      # Look for headers and libraries in the target's sysroot and in our own
      # prefix, but run build tools from the host.
      set(CMAKE_FIND_ROOT_PATH #{roots.join(';')})
      set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
      set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
      set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
      set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
    CMAKE
  end

  def compile(recipe)
    source = source_dir(recipe)
    build = "#{source}-build"

    unless Dir.exist?(source)
      FileUtils.mkdir_p(build_root)
      # Plain `xf`, not `xzf`: GNU tar, bsdtar and mingw's tar all sniff the
      # compression, which is what lets gzip and xz recipes share this path.
      run('tar', 'xf', tarball(recipe), '-C', build_root)
    end

    puts "Building #{recipe[:name]} #{recipe[:version]} for #{target}"
    run('cmake', '-S', source, '-B', build, *common_flags, *recipe[:flags])
    run('cmake', '--build', build, '--parallel', Etc.nprocessors.to_s)
    run('cmake', '--install', build)
  end

  def run(*command)
    return if system(*command)

    raise "Command failed: #{command.join(' ')}\n\n#{TOOLCHAIN_HINT}"
  end
end
