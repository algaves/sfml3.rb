#ifndef SFML_RB_CORE_SFML_H
#define SFML_RB_CORE_SFML_H

#include <CSFML/Config.h>

#if CSFML_VERSION_MAJOR < 3
#error "This extension requires CSFML 3. Run `rake ports` to build it, or upgrade your system CSFML."
#endif

#include <CSFML/Graphics.h>
#include <CSFML/Window.h>
#include <CSFML/Audio.h>
#include <CSFML/Network.h>
#include <CSFML/System.h>
#include <CSFML/System/Vector2.h>
#include <CSFML/Graphics/RenderStates.h>

#endif //SFML_RB_CORE_SFML_H
