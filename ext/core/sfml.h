#ifndef SFML_RB_CORE_SFML_H
#define SFML_RB_CORE_SFML_H

#include <CSFML/Config.h>

#if CSFML_VERSION_MAJOR < 3
#error                                                                                             \
    "This extension requires CSFML 3. Run `rake ports` to build it, or upgrade your system CSFML."
#endif

// The Emscripten/WebAssembly port (SFML_RB_WASM) only compiles the System and
// Audio bindings, so just their umbrellas are included there. The full set
// would not even parse: CSFML 3.0.0 declares sfWindowHandle
// (Window/WindowHandle.h) only for Windows/X11/macOS, so under Emscripten the
// umbrellas' RenderWindow.h references a type that was never defined. ext.c
// guards the matching Init_ calls (see SFML_RB_WASM).
#ifndef SFML_RB_WASM
#include <CSFML/Graphics.h>
#include <CSFML/Window.h>
#include <CSFML/Audio.h>
#include <CSFML/Network.h>
#include <CSFML/System.h>
#include <CSFML/System/Vector2.h>
#include <CSFML/Graphics/RenderStates.h>
#else
#include <CSFML/Audio.h>
#include <CSFML/System.h>
#include <CSFML/System/Vector2.h>
#endif

#endif // SFML_RB_CORE_SFML_H
