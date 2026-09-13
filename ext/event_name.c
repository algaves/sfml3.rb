#include "ext/klass/event/event_name.h"

#include <stddef.h>

#include "ext/sfml.h"

/* Indexed by sfEventType rather than by position. The CSFML 2.x table was
   positional, which broke silently under CSFML 3: the enum still has 24
   entries, but MouseWheelMoved was removed and MouseMovedRaw added, shifting
   every name from index 7 onward without any bounds error. */
static const char *events[] = {
        [sfEvtClosed] = "closed",
        [sfEvtResized] = "resized",
        [sfEvtFocusLost] = "lost-focus",
        [sfEvtFocusGained] = "gained-focus",
        [sfEvtTextEntered] = "text-entered",
        [sfEvtKeyPressed] = "key-pressed",
        [sfEvtKeyReleased] = "key-released",
        [sfEvtMouseWheelScrolled] = "mouse-wheel-scrolled",
        [sfEvtMouseButtonPressed] = "mouse-button-pressed",
        [sfEvtMouseButtonReleased] = "mouse-button-released",
        [sfEvtMouseMoved] = "mouse-moved",
        [sfEvtMouseMovedRaw] = "mouse-moved-raw",
        [sfEvtMouseEntered] = "mouse-entered",
        [sfEvtMouseLeft] = "mouse-left",
        [sfEvtJoystickButtonPressed] = "joystick-button-pressed",
        [sfEvtJoystickButtonReleased] = "joystick-button-released",
        [sfEvtJoystickMoved] = "joystick-moved",
        [sfEvtJoystickConnected] = "joystick-connected",
        [sfEvtJoystickDisconnected] = "joystick-disconnected",
        [sfEvtTouchBegan] = "touch-began",
        [sfEvtTouchMoved] = "touch-moved",
        [sfEvtTouchEnded] = "touch-ended",
        [sfEvtSensorChanged] = "sensor-changed"
};

#define LENGTH_EVENTS (sizeof(events) / sizeof(events[0]))

const char *get_event_name(size_t name) {
    if (name < LENGTH_EVENTS && events[name] != NULL) {
        return events[name];
    }

    return "";
}
