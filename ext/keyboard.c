#include "ext/klass/keyboard.h"

#include <stddef.h>
#include <string.h>

#include "ext/sfml.h"

/* Indexed by sfKeyCode rather than by position: if upstream reorders or
   removes a key, this fails to compile instead of silently renaming every
   key after the change. sfKeyUnknown is -1 and so has no entry. */
static const char *keys[] = {
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

int find_key(const char *name) {
    size_t i;

    for (i = 0; i < LENGTH_KEYS; i++) {
        if (keys[i] != NULL && strcmp(keys[i], name) == 0) {
            return (int) i;
        }
    }

    return -1;
}

const char *get_key_event(unsigned int key) {
    if (key < LENGTH_KEYS && keys[key] != NULL) {
        return keys[key];
    }

    return "";
}
