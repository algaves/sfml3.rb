#include "window/keyboard.h"

#include <stddef.h>
#include <ruby.h>
#include <string.h>

#include "core/exceptions.h"
#include "core/macros.h"
#include "core/sfml.h"

/* Indexed by sfKeyCode rather than by position: if upstream reorders or
   removes a key, this fails to compile instead of silently renaming every
   key after the change. sfKeyUnknown is -1 and so has no entry. */
static const char* keys[] = {
    [sfKeyA] = "a",
    [sfKeyB] = "b",
    [sfKeyC] = "c",
    [sfKeyD] = "d",
    [sfKeyE] = "e",
    [sfKeyF] = "f",
    [sfKeyG] = "g",
    [sfKeyH] = "h",
    [sfKeyI] = "i",
    [sfKeyJ] = "j",
    [sfKeyK] = "k",
    [sfKeyL] = "l",
    [sfKeyM] = "m",
    [sfKeyN] = "n",
    [sfKeyO] = "o",
    [sfKeyP] = "p",
    [sfKeyQ] = "q",
    [sfKeyR] = "r",
    [sfKeyS] = "s",
    [sfKeyT] = "t",
    [sfKeyU] = "u",
    [sfKeyV] = "v",
    [sfKeyW] = "w",
    [sfKeyX] = "x",
    [sfKeyY] = "y",
    [sfKeyZ] = "z",
    [sfKeyNum0] = "num0",
    [sfKeyNum1] = "num1",
    [sfKeyNum2] = "num2",
    [sfKeyNum3] = "num3",
    [sfKeyNum4] = "num4",
    [sfKeyNum5] = "num5",
    [sfKeyNum6] = "num6",
    [sfKeyNum7] = "num7",
    [sfKeyNum8] = "num8",
    [sfKeyNum9] = "num9",
    [sfKeyEscape] = "escape",
    [sfKeyLControl] = "LControl",
    [sfKeyLShift] = "LShift",
    [sfKeyLAlt] = "LAlt",
    [sfKeyLSystem] = "LSystem",
    [sfKeyRControl] = "RControl",
    [sfKeyRShift] = "RShift",
    [sfKeyRAlt] = "RAlt",
    [sfKeyRSystem] = "RSystem",
    [sfKeyMenu] = "Menu",
    [sfKeyLBracket] = "LBracket",
    [sfKeyRBracket] = "RBracket",
    [sfKeySemicolon] = "Semicolon",
    [sfKeyComma] = "Comma",
    [sfKeyPeriod] = "Period",
    [sfKeyApostrophe] = "Quote",
    [sfKeySlash] = "Slash",
    [sfKeyBackslash] = "Backslash",
    [sfKeyGrave] = "Tilde",
    [sfKeyEqual] = "Equal",
    [sfKeyHyphen] = "Hyphen",
    [sfKeySpace] = "Space",
    [sfKeyEnter] = "Enter",
    [sfKeyBackspace] = "Backspace",
    [sfKeyTab] = "Tab",
    [sfKeyPageUp] = "PageUp",
    [sfKeyPageDown] = "PageDown",
    [sfKeyEnd] = "End",
    [sfKeyHome] = "Home",
    [sfKeyInsert] = "Insert",
    [sfKeyDelete] = "Delete",
    [sfKeyAdd] = "Add",
    [sfKeySubtract] = "Subtract",
    [sfKeyMultiply] = "Multiply",
    [sfKeyDivide] = "Divide",
    [sfKeyLeft] = "Left",
    [sfKeyRight] = "Right",
    [sfKeyUp] = "Up",
    [sfKeyDown] = "Down",
    [sfKeyNumpad0] = "Numpad0",
    [sfKeyNumpad1] = "Numpad1",
    [sfKeyNumpad2] = "Numpad2",
    [sfKeyNumpad3] = "Numpad3",
    [sfKeyNumpad4] = "Numpad4",
    [sfKeyNumpad5] = "Numpad5",
    [sfKeyNumpad6] = "Numpad6",
    [sfKeyNumpad7] = "Numpad7",
    [sfKeyNumpad8] = "Numpad8",
    [sfKeyNumpad9] = "Numpad9",
    [sfKeyF1] = "f1",
    [sfKeyF2] = "f2",
    [sfKeyF3] = "f3",
    [sfKeyF4] = "f4",
    [sfKeyF5] = "f5",
    [sfKeyF6] = "f6",
    [sfKeyF7] = "f7",
    [sfKeyF8] = "f8",
    [sfKeyF9] = "f9",
    [sfKeyF10] = "f10",
    [sfKeyF11] = "f11",
    [sfKeyF12] = "f12",
    [sfKeyF13] = "f13",
    [sfKeyF14] = "f14",
    [sfKeyF15] = "f15",
    [sfKeyPause] = "pause",
};

#define LENGTH_KEYS (sizeof(keys) / sizeof(keys[0]))

int find_key(const char* name) {
    size_t i;

    for (i = 0; i < LENGTH_KEYS; i++) {
        if (keys[i] != NULL && strcmp(keys[i], name) == 0) {
            return (int)i;
        }
    }

    return -1;
}

const char* get_key_event(unsigned int key) {
    if (key < LENGTH_KEYS && keys[key] != NULL) {
        return keys[key];
    }

    return "";
}

static int Keyboard_key_code(VALUE rb_key) {
    int code;

    if (SYMBOL_P(rb_key)) {
        const char* name = rb_id2name(SYM2ID(rb_key));

        code = find_key(name);

        if (code < 0) {
            rb_raise(rb_eArgError, "unknown key: %s", name);
        }

        return code;
    }

    if (RB_TYPE_P(rb_key, T_STRING)) {
        const char* name = StringValueCStr(rb_key);

        code = find_key(name);

        if (code < 0) {
            rb_raise(rb_eArgError, "unknown key: %s", name);
        }

        return code;
    }

    return NUM2INT(rb_key);
}

/* call-seq:
 *   pressed?(key) -> true or false
 *
 * +key+ may be a key Symbol (e.g. +:a+, +:Enter+, +:LShift+), a String key
 * name, or an Integer key code.
 *
 * @return [Boolean]
 * @raise [ArgumentError] if +key+ is a Symbol/String naming an unknown key
 */
static VALUE Keyboard_pressed_p(VALUE module, VALUE rb_key) {
    return BOOL2RB(sfKeyboard_isKeyPressed((sfKeyCode)Keyboard_key_code(rb_key)));
}

/* call-seq:
 *   scancode_pressed?(scancode) -> true or false
 *
 * Unlike #pressed?, +scancode+ identifies a physical key position rather
 * than the character it currently produces.
 *
 * @return [Boolean]
 */
static VALUE Keyboard_scancode_pressed_p(VALUE module, VALUE rb_scancode) {
    return BOOL2RB(sfKeyboard_isScancodePressed((sfScancode)NUM2INT(rb_scancode)));
}

/* call-seq:
 *   localize(scancode) -> Symbol
 *
 * @return [Symbol] the key that +scancode+ produces on the current keyboard
 *   layout
 */
static VALUE Keyboard_localize(VALUE module, VALUE rb_scancode) {
    return ID2SYM(rb_intern(
        get_key_event((unsigned int)sfKeyboard_localize((sfScancode)NUM2INT(rb_scancode)))));
}

/* call-seq:
 *   delocalize(key) -> Integer
 *
 * @return [Integer] the scancode that produces +key+ on the current keyboard
 *   layout
 * @raise [ArgumentError] if +key+ is a Symbol/String naming an unknown key
 */
static VALUE Keyboard_delocalize(VALUE module, VALUE rb_key) {
    return INT2NUM(sfKeyboard_delocalize((sfKeyCode)Keyboard_key_code(rb_key)));
}

/* call-seq:
 *   description(scancode) -> String
 *
 * @return [String] a human-readable, localized name for +scancode+
 */
static VALUE Keyboard_description(VALUE module, VALUE rb_scancode) {
    const char* description = sfKeyboard_getDescription((sfScancode)NUM2INT(rb_scancode));

    return rb_str_new_cstr(description != NULL ? description : "");
}

/* call-seq:
 *   virtual_keyboard_visible=(value) -> true or false
 *
 * Shows or hides the on-screen keyboard, where the platform provides one.
 *
 * @return [Boolean] +value+
 */
static VALUE Keyboard_set_virtual_keyboard_visible(VALUE module, VALUE rb_visible) {
    sfKeyboard_setVirtualKeyboardVisible(RTEST(rb_visible));
    return rb_visible;
}

/* Document-module: SFML::Keyboard
 * Real-time keyboard state and key/scancode lookups.
 */
void Init_Keyboard(VALUE rb_mSFML) {
    VALUE rb_mKeyboard = rb_define_module_under(rb_mSFML, "Keyboard");

    rb_define_module_function(rb_mKeyboard, "pressed?", Keyboard_pressed_p, 1);
    rb_define_module_function(rb_mKeyboard, "scancode_pressed?", Keyboard_scancode_pressed_p, 1);
    rb_define_module_function(rb_mKeyboard, "localize", Keyboard_localize, 1);
    rb_define_module_function(rb_mKeyboard, "delocalize", Keyboard_delocalize, 1);
    rb_define_module_function(rb_mKeyboard, "description", Keyboard_description, 1);
    rb_define_module_function(
        rb_mKeyboard, "virtual_keyboard_visible=", Keyboard_set_virtual_keyboard_visible, 1);
}
