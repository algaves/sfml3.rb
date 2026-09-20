# frozen_string_literal: true

require 'mkmf'
require_relative 'ports'

# The Emscripten/WebAssembly port (see WASM.md). ruby.wasm builds the extension
# statically into ruby.wasm: mkmf's `make static` only archives the objects, so
# SFML/CSFML are needed here for their headers alone. The wasm static archives
# themselves are injected into the final emcc link by rbwasm through
# RUBY_WASM_EMCC_LDFLAGS (or a pre-created link.filelist); linking them from
# this Makefile would only bake host toolchain search paths that the final link
# never uses. SFML_WASM_PREFIX must point at a prefix that has both the
# CSFML and SFML wasm headers installed (that is, the merged SFML+CSFML wasm
# prefix produced by the port build).
WASM_PREFIX = ENV.fetch('SFML_WASM_PREFIX', nil)

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
# on FreeType and the Ogg/Vorbis/FLAC codecs, and all of them depend on the
# target's OS libraries. This mirrors the INTERFACE_LINK_LIBRARIES that SFML's
# own CMake config exports. Within the codecs, vorbisfile/vorbisenc depend on
# vorbis, which depends on ogg, and FLAC stands alone; all of them must follow
# sfml-audio so the linker has already seen the references.
#
# These are named by full path rather than -l. mkmf puts the host's -L/usr/lib64
# ahead of the ports prefix, so a plain -lfreetype resolves to the host's shared
# library and the built extension silently gains a runtime dependency on it. A
# full path to the .a is used as-is by every linker (GNU ld, mingw, ld64) and
# cannot be shadowed.
VENDORED_LIBS = %w[
  csfml-graphics-s csfml-window-s csfml-system-s csfml-audio-s csfml-network-s
  sfml-graphics-s sfml-window-s sfml-system-s sfml-audio-s sfml-network-s
  freetype
  vorbisfile vorbisenc vorbis ogg
  FLAC
].freeze

SYSTEM_CSFML_LIBS = %w[
  csfml-graphics csfml-window csfml-system csfml-audio csfml-network
].freeze

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
    *VENDORED_LIBS.map { |l| File.join(Ports.prefix, 'lib', "lib#{l}.a") },
    *Ports.libs.map { |l| "-l#{l}" },
    *Ports.cxx_runtime
  ].join(' ')
end

def use_wasm_ports
  $INCFLAGS = "-I#{WASM_PREFIX}/include #{$INCFLAGS}"
  $defs.push('-DSFML_RB_WASM')
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
#   1. SFML_WASM_PREFIX -> Emscripten/WebAssembly cross build (headers only;
#      the SFML/CSFML archives come from the rbwasm final link)
#   2. --enable-system-libraries -> link a system CSFML 3 (offline-capable)
#   3. an already-built ports prefix -> reuse it (fast path for development,
#      and the path cross builds take, since rake-compiler-dock builds the
#      ports before invoking the extension build)
#   4. otherwise -> download and build FreeType, SFML 3 and CSFML 3, then link
if WASM_PREFIX
  use_wasm_ports
elsif enable_config('system-libraries', ENV.fetch('SFML_USE_SYSTEM_LIBRARIES', nil))
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

# The sources live in per-subsystem directories (core/, system/, window/,
# graphics/) but mkmf only globs the top level of $srcdir, so the list has to be
# handed to it. Objects still land flat in the build directory: mkmf derives
# $objs from File.basename, and make finds each source through VPATH -- which is
# why no mkdir rules are needed for the object tree.
#
# That flattening also means every .c basename must be unique across the whole
# tree. mkmf enforces it, aborting with "source files duplication", so a
# collision fails the build loudly rather than dropping a file.
sources = if WASM_PREFIX
            # The Emscripten port only builds the System and Audio bindings: upstream
            # SFML 3.0.2/CSFML 3.0.0 have no Emscripten backend, so the window/graphics/
            # network archives do not exist to link against. The audio-thread files are
            # dropped too -- they wrap pthreads via core/foreign_thread.c, and there is no
            # pthread support in the browser build; SFML::Sound/Music also pull the
            # effect-processor pool (see ext.c's SFML_RB_WASM guards). ext.c (the entry
            # point, Init_sfml_ext) lives at the top level and is not covered by any
            # subsystem glob, so it is listed explicitly.
            [File.join($srcdir, 'ext.c')] +
              (Dir.glob("#{$srcdir}/core/**/*.c") +
               Dir.glob("#{$srcdir}/system/**/*.c") +
               Dir.glob("#{$srcdir}/audio/**/*.c")) -
              %w[
                core/foreign_thread.c
                audio/effect_processor.c
                audio/sound.c
                audio/music.c
                audio/sound_stream.c
                audio/sound_buffer_recorder.c
                audio/sound_recorder.c
              ].map { |f| File.join($srcdir, f) }
          else
            Dir.glob("#{$srcdir}/**/*.c")
          end
$srcs = sources
$VPATH.concat(
  sources.map { |file| File.dirname(file) }
         .uniq
         .reject { |dir| dir == $srcdir }
         .map { |dir| dir.sub(/\A#{Regexp.escape($srcdir)}/, '$(srcdir)') }
)

create_makefile 'sfml/sfml_ext'
