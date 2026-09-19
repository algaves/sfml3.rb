#include "window/event.h"

#include <ruby.h>
#include <stdio.h>
#include <stdlib.h>

#include "window/event_name.h"
#include "window/keyboard.h"
#include "window/input_enums.h"
#include "system/vec2.h"
#include "system/vec3.h"
#include "core/macros.h"
#include "core/sfml.h"

static VALUE rb_cEvent;

static sfEvent* Event_create() {
    sfEvent* event = malloc(sizeof(sfEvent));

    if (event == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate event");
    }

    return event;
}

static void Event_free(void* ptr) {
    free(ptr);
}

static const rb_data_type_t Event_data_type = {
    .wrap_struct_name = "SFML::Event",
    .function = {.dmark = NULL, .dfree = Event_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

/* call-seq:
 *   Event.new -> Event
 *
 * Creates an empty, uninitialized event.
 *
 * @return [Event]
 */
static VALUE Event_new(VALUE klass) {
    VALUE self;
    sfEvent* event;

    event = Event_create();
    self = TypedData_Wrap_Struct(klass, &Event_data_type, event);

    rb_obj_call_init(self, 0, NULL);

    return self;
}

/* call-seq: initialize -> self
 *
 * Does nothing; instances are built internally by the event-polling methods.
 *
 * @private No-op; objects of this class are only ever constructed
 *   internally (e.g. by #poll_event!).
 * @return [self]
 */
static VALUE Event_init(VALUE self) {
    return self;
}

/* call-seq: type -> String
 *
 * Returns the event's kind as a String, such as +"closed"+ or +"key-pressed"+.
 *
 * @return [String] the event's kind, e.g. +"closed"+, +"key-pressed"+,
 *   +"mouse-moved"+ (see SFML::Window#poll_event!)
 */
static VALUE Event_type(VALUE self) {
    return rb_str_new2(get_event_name(Get_Event_Struct(self)->type));
}

/* call-seq: size -> Vector2
 *
 * Returns the new size carried by a +"resized"+ event.
 *
 * @return [Vector2] the new size, for a +"resized"+ event
 */
static VALUE Event_get_size(VALUE self) {
    sfVector2u size = Get_Event_Struct(self)->size.size;

    return vec2f_to_rb((sfVector2f){(float)size.x, (float)size.y});
}

/* call-seq: key -> Hash
 *
 * Returns the key descriptor Hash for a key press or release event.
 *
 * @return [Hash] +:code+, +:scancode+, +:alt+, +:control+, +:shift+,
 *   +:system+, for a +"key-pressed"+/+"key-released"+ event
 */
static VALUE Event_get_key(VALUE self) {
    sfKeyEvent* event = &Get_Event_Struct(self)->key;
    VALUE hash = rb_hash_new();

    rb_hash_aset(hash, ID2SYM(rb_intern("code")), ID2SYM(rb_intern(get_key_event(event->code))));
    rb_hash_aset(hash, ID2SYM(rb_intern("scancode")),
                 rb_str_new2(sfKeyboard_getDescription(event->scancode)));
    rb_hash_aset(hash, ID2SYM(rb_intern("alt")), BOOL2RB(event->alt));
    rb_hash_aset(hash, ID2SYM(rb_intern("control")), BOOL2RB(event->control));
    rb_hash_aset(hash, ID2SYM(rb_intern("shift")), BOOL2RB(event->shift));
    rb_hash_aset(hash, ID2SYM(rb_intern("system")), BOOL2RB(event->system));

    return hash;
}

static VALUE unicode_to_utf8(uint32_t code) {
    char buffer[5];
    int length = 0;

    if (code < 0x80) {
        buffer[length++] = (char)code;
    } else if (code < 0x800) {
        buffer[length++] = (char)(0xC0 | (code >> 6));
        buffer[length++] = (char)(0x80 | (code & 0x3F));
    } else if (code < 0x10000) {
        buffer[length++] = (char)(0xE0 | (code >> 12));
        buffer[length++] = (char)(0x80 | ((code >> 6) & 0x3F));
        buffer[length++] = (char)(0x80 | (code & 0x3F));
    } else {
        buffer[length++] = (char)(0xF0 | (code >> 18));
        buffer[length++] = (char)(0x80 | ((code >> 12) & 0x3F));
        buffer[length++] = (char)(0x80 | ((code >> 6) & 0x3F));
        buffer[length++] = (char)(0x80 | (code & 0x3F));
    }

    return rb_utf8_str_new(buffer, length);
}

/* call-seq: text -> String
 *
 * Returns the entered character for a +"text-entered"+ event.
 *
 * @return [String] the entered character, decoded from Unicode, for a
 *   +"text-entered"+ event
 */
static VALUE Event_get_text(VALUE self) {
    return unicode_to_utf8(Get_Event_Struct(self)->text.unicode);
}

/* call-seq: mouse_move -> Vector2
 *
 * Returns the new cursor position for a +"mouse-moved"+ event.
 *
 * @return [Vector2] the new cursor position, for a +"mouse-moved"+ event
 */
static VALUE Event_get_mouse_move(VALUE self) {
    sfVector2i position = Get_Event_Struct(self)->mouseMove.position;

    return vec2f_to_rb((sfVector2f){(float)position.x, (float)position.y});
}

/* call-seq: mouse_move_raw -> Vector2
 *
 * Returns the unfiltered relative motion for a +"mouse-moved-raw"+ event.
 *
 * @return [Vector2] unfiltered relative motion, for a +"mouse-moved-raw"+
 *   event
 */
static VALUE Event_get_mouse_move_raw(VALUE self) {
    sfVector2i delta = Get_Event_Struct(self)->mouseMoveRaw.delta;

    return vec2f_to_rb((sfVector2f){(float)delta.x, (float)delta.y});
}

/* call-seq: mouse_button -> Hash
 *
 * Returns the button descriptor Hash for a mouse press or release event.
 *
 * @return [Hash] +:button+, +:position+, for a
 *   +"mouse-button-pressed"+/+"mouse-button-released"+ event
 */
static VALUE Event_get_mouse_button(VALUE self) {
    sfMouseButtonEvent* event = &Get_Event_Struct(self)->mouseButton;
    VALUE hash = rb_hash_new();

    rb_hash_aset(hash, ID2SYM(rb_intern("button")),
                 ID2SYM(rb_intern(mouse_button_name(event->button))));
    rb_hash_aset(hash, ID2SYM(rb_intern("position")),
                 vec2f_to_rb((sfVector2f){(float)event->position.x, (float)event->position.y}));

    return hash;
}

/* call-seq: mouse_wheel_scroll -> Hash
 *
 * Returns the wheel descriptor Hash for a +"mouse-wheel-scrolled"+ event.
 *
 * @return [Hash] +:wheel+, +:delta+, +:position+, for a
 *   +"mouse-wheel-scrolled"+ event
 */
static VALUE Event_get_mouse_wheel_scroll(VALUE self) {
    sfMouseWheelScrollEvent* event = &Get_Event_Struct(self)->mouseWheelScroll;
    VALUE hash = rb_hash_new();

    rb_hash_aset(hash, ID2SYM(rb_intern("wheel")),
                 ID2SYM(rb_intern(mouse_wheel_name(event->wheel))));
    rb_hash_aset(hash, ID2SYM(rb_intern("delta")), DBL2NUM(event->delta));
    rb_hash_aset(hash, ID2SYM(rb_intern("position")),
                 vec2f_to_rb((sfVector2f){(float)event->position.x, (float)event->position.y}));

    return hash;
}

/* call-seq: joystick_move -> Hash
 *
 * Returns the descriptor Hash for a +"joystick-moved"+ event.
 *
 * @return [Hash] +:joystick_id+, +:axis+, +:position+, for a
 *   +"joystick-moved"+ event
 */
static VALUE Event_get_joystick_move(VALUE self) {
    sfJoystickMoveEvent* event = &Get_Event_Struct(self)->joystickMove;
    VALUE hash = rb_hash_new();

    rb_hash_aset(hash, ID2SYM(rb_intern("joystick_id")), UINT2NUM(event->joystickId));
    rb_hash_aset(hash, ID2SYM(rb_intern("axis")),
                 ID2SYM(rb_intern(joystick_axis_name(event->axis))));
    rb_hash_aset(hash, ID2SYM(rb_intern("position")), DBL2NUM(event->position));

    return hash;
}

/* call-seq: joystick_button -> Hash
 *
 * Returns the descriptor Hash for a joystick button press or release event.
 *
 * @return [Hash] +:joystick_id+, +:button+, for a
 *   +"joystick-button-pressed"+/+"joystick-button-released"+ event
 */
static VALUE Event_get_joystick_button(VALUE self) {
    sfJoystickButtonEvent* event = &Get_Event_Struct(self)->joystickButton;
    VALUE hash = rb_hash_new();

    rb_hash_aset(hash, ID2SYM(rb_intern("joystick_id")), UINT2NUM(event->joystickId));
    rb_hash_aset(hash, ID2SYM(rb_intern("button")), UINT2NUM(event->button));

    return hash;
}

/* call-seq: joystick_connect -> Hash
 *
 * Returns the descriptor Hash for a joystick connect or disconnect event.
 *
 * @return [Hash] +:joystick_id+, +:connected+, for a
 *   +"joystick-connected"+/+"joystick-disconnected"+ event
 */
static VALUE Event_get_joystick_connect(VALUE self) {
    sfEvent* event = Get_Event_Struct(self);
    VALUE hash = rb_hash_new();

    rb_hash_aset(hash, ID2SYM(rb_intern("joystick_id")),
                 UINT2NUM(event->joystickConnect.joystickId));
    rb_hash_aset(hash, ID2SYM(rb_intern("connected")),
                 BOOL2RB(event->type == sfEvtJoystickConnected));

    return hash;
}

/* call-seq: touch -> Hash
 *
 * Returns the descriptor Hash for a touch begin, move or end event.
 *
 * @return [Hash] +:finger+, +:position+, for a
 *   +"touch-began"+/+"touch-moved"+/+"touch-ended"+ event
 */
static VALUE Event_get_touch(VALUE self) {
    sfTouchEvent* event = &Get_Event_Struct(self)->touch;
    VALUE hash = rb_hash_new();

    rb_hash_aset(hash, ID2SYM(rb_intern("finger")), UINT2NUM(event->finger));
    rb_hash_aset(hash, ID2SYM(rb_intern("position")),
                 vec2f_to_rb((sfVector2f){(float)event->position.x, (float)event->position.y}));

    return hash;
}

/* call-seq: sensor -> Hash
 *
 * Returns the descriptor Hash for a +"sensor-changed"+ event.
 *
 * @return [Hash] +:type+, +:value+, for a +"sensor-changed"+ event
 */
static VALUE Event_get_sensor(VALUE self) {
    sfSensorEvent* event = &Get_Event_Struct(self)->sensor;
    VALUE hash = rb_hash_new();

    rb_hash_aset(hash, ID2SYM(rb_intern("type")),
                 ID2SYM(rb_intern(sensor_type_name(event->sensorType))));
    rb_hash_aset(hash, ID2SYM(rb_intern("value")), vec3f_to_rb(event->value));

    return hash;
}

/* Document-class: SFML::Event
 * A window or input event, polled from Window#poll_event!/#wait_event!.
 *
 * #type tells which of the accessors below is meaningful for this
 * particular event; the others return stale or default data.
 */
void Init_Event(VALUE rb_mSFML) {
    rb_cEvent = rb_define_class_under(rb_mSFML, "Event", rb_cObject);

    rb_define_singleton_method(rb_cEvent, "new", Event_new, 0);

    rb_define_method(rb_cEvent, "initialize", Event_init, 0);

    rb_define_method(rb_cEvent, "type", Event_type, 0);
    rb_define_method(rb_cEvent, "size", Event_get_size, 0);
    rb_define_method(rb_cEvent, "key", Event_get_key, 0);
    rb_define_method(rb_cEvent, "text", Event_get_text, 0);
    rb_define_method(rb_cEvent, "mouse_move", Event_get_mouse_move, 0);
    rb_define_method(rb_cEvent, "mouse_move_raw", Event_get_mouse_move_raw, 0);
    rb_define_method(rb_cEvent, "mouse_button", Event_get_mouse_button, 0);
    rb_define_method(rb_cEvent, "mouse_wheel_scroll", Event_get_mouse_wheel_scroll, 0);
    rb_define_method(rb_cEvent, "mouse_wheel", Event_get_mouse_wheel_scroll, 0);
    rb_define_method(rb_cEvent, "joystick_move", Event_get_joystick_move, 0);
    rb_define_method(rb_cEvent, "joystick_button", Event_get_joystick_button, 0);
    rb_define_method(rb_cEvent, "joystick_connect", Event_get_joystick_connect, 0);
    rb_define_method(rb_cEvent, "touch", Event_get_touch, 0);
    rb_define_method(rb_cEvent, "sensor", Event_get_sensor, 0);
}

sfEvent* Get_Event_Struct(VALUE self) {
    sfEvent* ptr;
    TypedData_Get_Struct(self, sfEvent, &Event_data_type, ptr);
    return ptr;
}

VALUE Get_Klass_Event() {
    return rb_cEvent;
}
