cmake_minimum_required(VERSION 3.22)

# CXX is enabled purely for linking: every source here is C, but SFML is C++, so
# the link step has to go through the C++ driver to pull in the C++ runtime.
# Without it the module builds fine and then fails at require time with an
# undefined __cxxabiv1 vtable symbol.
project(sfml_rb C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Mirrors the mkmf build in ext/extconf.rb: same sources, same vendored CSFML 3
# prefix, same resulting ext.so. `rake compile` stays the build of record; this
# exists so the extension can be indexed, built and debugged from an IDE, and as
# a second opinion that catches link errors mkmf would only surface later.
#
# Run `rake ports` before configuring.

find_program(RUBY_EXECUTABLE ruby REQUIRED)

# Query the running interpreter instead of vendoring header stubs, which is what
# the old include/ruby/*.h did -- those shadowed the real headers and made the
# IDE index against fakes.
function(ruby_config out_var key)
    execute_process(
            COMMAND "${RUBY_EXECUTABLE}" -rrbconfig -e "print RbConfig::CONFIG['${key}']"
            OUTPUT_VARIABLE value
            RESULT_VARIABLE status
    )

    if (NOT status EQUAL 0)
        message(FATAL_ERROR "Could not read RbConfig::CONFIG['${key}'] from ${RUBY_EXECUTABLE}")
    endif ()

    set(${out_var} "${value}" PARENT_SCOPE)
endfunction()

ruby_config(RUBY_HDR_DIR rubyhdrdir)
ruby_config(RUBY_ARCH_HDR_DIR rubyarchhdrdir)
ruby_config(RUBY_HOST host)

set(PORTS_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/ports/${RUBY_HOST}")

if (NOT EXISTS "${PORTS_PREFIX}/include/CSFML")
    message(FATAL_ERROR
            "Vendored CSFML 3 not found at ${PORTS_PREFIX}\n"
            "Run `rake ports` to build SFML and CSFML from source first.")
endif ()

# SFML installs a CMake config package, so its imported targets carry the
# transitive X11/GL/udev/freetype dependencies. CSFML installs no config
# package, so its three static archives are located by hand.
list(APPEND CMAKE_PREFIX_PATH "${PORTS_PREFIX}")

# `rake ports` builds SFML with BUILD_SHARED_LIBS=OFF, and SFMLConfig refuses to
# resolve a static install unless it is told to look for one.
set(SFML_STATIC_LIBRARIES ON)
find_package(SFML 3 COMPONENTS Graphics Window System Audio Network REQUIRED)

foreach (module graphics window system audio network)
    find_library(CSFML_${module}_LIBRARY
            NAMES csfml-${module}-s csfml-${module}
            PATHS "${PORTS_PREFIX}/lib"
            NO_DEFAULT_PATH
            REQUIRED
    )
    list(APPEND CSFML_LIBRARIES "${CSFML_${module}_LIBRARY}")
endforeach ()

# Globbed rather than listed by hand: the previous explicit list was a single
# 900-character line that had already drifted from the tree.
file(GLOB_RECURSE ext_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ext/*.c")
file(GLOB_RECURSE ext_headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ext/*.h")

add_library(sfml_rb MODULE ${ext_sources} ${ext_headers})

# Match what mkmf produces: sfml_ext.so, not libsfml_rb.so.
set_target_properties(sfml_rb PROPERTIES
        PREFIX ""
        OUTPUT_NAME sfml_ext
        LINKER_LANGUAGE CXX
)

target_include_directories(sfml_rb PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/ext"
        "${PORTS_PREFIX}/include"
        "${RUBY_HDR_DIR}"
        "${RUBY_ARCH_HDR_DIR}"
)

# CSFML first: static archives must precede the SFML symbols they reference.
target_link_libraries(sfml_rb PRIVATE
        ${CSFML_LIBRARIES}
        SFML::Graphics
        SFML::Window
        SFML::System
        SFML::Audio
        SFML::Network
)
