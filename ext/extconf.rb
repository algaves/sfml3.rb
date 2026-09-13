require 'mkmf'
require_relative 'auxlib'
require_relative 'ports'

# Link order matters for static archives: CSFML depends on SFML, and SFML
# depends on the system X11/GL/udev stack. This mirrors the
# INTERFACE_LINK_LIBRARIES that SFML's own CMake config exports.
VENDORED_LIBS = %w[
  csfml-graphics-s csfml-window-s csfml-system-s
  sfml-graphics-s sfml-window-s sfml-system-s
].freeze

VENDORED_SYSTEM_LIBS = %w[
  freetype GL X11 Xrandr Xcursor Xi udev pthread dl rt stdc++ m
].freeze

SYSTEM_LIBS = %w[csfml-graphics csfml-window csfml-system].freeze

def use_vendored_ports
  unless System.linux?
    abort "Building the vendored SFML currently supports Linux only.\n" \
          'Install CSFML 3 and reinstall with: gem install sfml3-rb -- --enable-system-libraries'
  end

  $INCFLAGS = "-I#{Ports::PREFIX}/include #{$INCFLAGS}"
  $LDFLAGS = "#{$LDFLAGS} -L#{Ports::PREFIX}/lib"
  $libs = [$libs, *(VENDORED_LIBS + VENDORED_SYSTEM_LIBS).map { |l| "-l#{l}" }].join(' ')
end

def use_system_csfml
  missing = SYSTEM_LIBS.reject { |l| have_library(l) }
  return if missing.empty?

  abort <<~MSG
    Could not find a system CSFML 3: #{missing.join(', ')}

    Install CSFML 3 development files, or drop --enable-system-libraries to let
    the gem download and build SFML 3 and CSFML 3 itself.

    Note that distro packages are often still CSFML 2.x, which this extension
    no longer supports.
  MSG
end

# Resolution order:
#   1. --enable-system-libraries -> link a system CSFML 3 (offline-capable)
#   2. an already-built ports prefix -> reuse it (fast path for development)
#   3. otherwise -> download and build SFML 3 + CSFML 3, then link that
if enable_config('system-libraries', ENV.fetch('SFML_USE_SYSTEM_LIBRARIES', nil))
  use_system_csfml
else
  Ports.build! unless Ports.built?
  use_vendored_ports
end

# Fails at configure time with a readable message rather than letting a CSFML 2.x
# header produce a wall of compile errors later.
unless try_compile(<<~C)
  #include <CSFML/Config.h>
  #if CSFML_VERSION_MAJOR < 3
  #error "CSFML 3 required"
  #endif
  int main(void) { return 0; }
C
  abort <<~MSG
    Could not compile against CSFML 3 headers (<CSFML/Config.h>).

    Either no CSFML is installed, or the one found is CSFML 2.x. Note that CSFML
    2.x installs its headers under SFML/ rather than CSFML/, and distro packages
    are frequently still 2.x -- so a successful library check above does not mean
    the version is right.

    Drop --enable-system-libraries to let the gem download and build CSFML 3 itself.
  MSG
end

create_makefile 'sfml/sfml_ext'
