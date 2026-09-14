require 'mkmf'
require_relative 'ports'

# Warnings are on everywhere: a user's build log is the only diagnostic anyone
# gets when an install fails on a platform we never tested.
$CFLAGS = "#{$CFLAGS} -Wall -Wextra -Wno-unused-parameter"

# Promoting them to errors is opt-in, and CI is the only caller. An unfamiliar
# compiler at `gem install` time will warn about things this code cannot
# predict, and a warning must never be what stops someone installing the gem.
# SFML_STRICT=1 is what keeps the TypedData migration from regressing: going
# back to Data_Wrap_Struct would reintroduce a deprecation warning and fail.
$CFLAGS = "#{$CFLAGS} -Werror=deprecated-declarations" if ENV['SFML_STRICT']

# Link order matters for static archives: CSFML depends on SFML, SFML depends
# on FreeType, and all of them depend on the target's OS libraries. This
# mirrors the INTERFACE_LINK_LIBRARIES that SFML's own CMake config exports.
VENDORED_LIBS = %w[
  csfml-graphics-s csfml-window-s csfml-system-s
  sfml-graphics-s sfml-window-s sfml-system-s
  freetype
].freeze

SYSTEM_CSFML_LIBS = %w[csfml-graphics csfml-window csfml-system].freeze

def use_vendored_ports
  $INCFLAGS = "-I#{Ports.prefix}/include #{$INCFLAGS}"
  $LDFLAGS = "#{$LDFLAGS} -L#{Ports.prefix}/lib"
  $defs.concat(Ports.defines)

  # A precompiled gem is loaded on machines that never had this toolchain, so
  # don't make it depend on the build host's libgcc.
  $LDFLAGS = "#{$LDFLAGS} -static-libgcc" if Ports.cross? && Ports.os != :darwin

  Ports.frameworks.each { |f| $LDFLAGS = "#{$LDFLAGS} -framework #{f}" }

  # The C++ runtime goes last: it has to resolve symbols left over from the
  # static SFML archives ahead of it.
  $libs = [
    $libs,
    *(VENDORED_LIBS + Ports.libs).map { |l| "-l#{l}" },
    *Ports.cxx_runtime
  ].join(' ')
end

def use_system_csfml
  missing = SYSTEM_CSFML_LIBS.reject { |l| have_library(l) }
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
#   2. an already-built ports prefix -> reuse it (fast path for development,
#      and the path cross builds take, since rake-compiler-dock builds the
#      ports before invoking the extension build)
#   3. otherwise -> download and build FreeType, SFML 3 and CSFML 3, then link
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
