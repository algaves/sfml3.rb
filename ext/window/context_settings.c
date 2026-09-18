#include "window/context_settings.h"

#include <ruby.h>
#include <stdlib.h>
#include <string.h>

#include "core/exceptions.h"
#include "core/macros.h"

typedef struct {
    sfContextSettings settings;
} ContextSettings;

static VALUE rb_cContextSettings;

static void ContextSettings_free(void* ptr) {
    free(ptr);
}

static const rb_data_type_t ContextSettings_data_type = {
    .wrap_struct_name = "SFML::ContextSettings",
    .function = {.dmark = NULL, .dfree = ContextSettings_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE ContextSettings_wrap(sfContextSettings settings) {
    ContextSettings* ptr = malloc(sizeof(ContextSettings));

    if (ptr == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate context settings");
    }

    ptr->settings = settings;

    return TypedData_Wrap_Struct(rb_cContextSettings, &ContextSettings_data_type, ptr);
}

static const char* context_flag_name(VALUE value) {
    if (SYMBOL_P(value)) {
        return rb_id2name(SYM2ID(value));
    }

    return StringValueCStr(value);
}

/* call-seq:
 *   ContextSettings.new(depth_bits = 0, stencil_bits = 0, antialiasing_level = 0, major_version =
 * 1, minor_version = 1, flags = :default, srgb_capable = false) -> ContextSettings
 *
 * @return [ContextSettings]
 * @raise [ArgumentError] if +flags+ contains an unknown attribute name
 */
static VALUE ContextSettings_new(int argc, VALUE* argv, VALUE klass) {
    sfContextSettings settings = {0, 0, 0, 1, 1, sfContextDefault, false};
    VALUE rb_depth, rb_stencil, rb_antialiasing, rb_major, rb_minor, rb_flags, rb_srgb;
    VALUE self;

    rb_scan_args(argc, argv, "07", &rb_depth, &rb_stencil, &rb_antialiasing, &rb_major, &rb_minor,
                 &rb_flags, &rb_srgb);

    if (!NIL_P(rb_depth)) {
        settings.depthBits = (unsigned int)NUM2UINT(rb_depth);
    }

    if (!NIL_P(rb_stencil)) {
        settings.stencilBits = (unsigned int)NUM2UINT(rb_stencil);
    }

    if (!NIL_P(rb_antialiasing)) {
        settings.antiAliasingLevel = (unsigned int)NUM2UINT(rb_antialiasing);
    }

    if (!NIL_P(rb_major)) {
        settings.majorVersion = (unsigned int)NUM2UINT(rb_major);
    }

    if (!NIL_P(rb_minor)) {
        settings.minorVersion = (unsigned int)NUM2UINT(rb_minor);
    }

    if (!NIL_P(rb_flags)) {
        uint32_t flags = 0;

        if (RB_INTEGER_TYPE_P(rb_flags)) {
            flags = (uint32_t)NUM2UINT(rb_flags);
        } else if (SYMBOL_P(rb_flags) || RB_TYPE_P(rb_flags, T_STRING)) {
            rb_flags = rb_ary_new_from_args(1, rb_flags);
        }

        if (RB_TYPE_P(rb_flags, T_ARRAY)) {
            for (long i = 0; i < RARRAY_LEN(rb_flags); i++) {
                VALUE entry = rb_ary_entry(rb_flags, i);
                const char* name = context_flag_name(entry);

                if (strcmp(name, "core") == 0) {
                    flags |= sfContextCore;
                } else if (strcmp(name, "debug") == 0) {
                    flags |= sfContextDebug;
                } else if (strcmp(name, "default") != 0) {
                    rb_raise(rb_eArgError, "unknown context attribute: %s", name);
                }
            }
        }

        settings.attributeFlags = flags;
    }

    if (!NIL_P(rb_srgb)) {
        settings.sRgbCapable = RTEST(rb_srgb);
    }

    self = ContextSettings_wrap(settings);

    return self;
}

#define CONTEXT_SETTINGS_UINT_ACCESSOR(name, field)                                                \
    static VALUE ContextSettings_get_##name(VALUE self) {                                          \
        return UINT2NUM(((ContextSettings*)Get_ContextSettings_Struct(self))->settings.field);     \
    }                                                                                              \
    static VALUE ContextSettings_set_##name(VALUE self, VALUE rb_value) {                          \
        rb_check_frozen(self);                                                                     \
        ((ContextSettings*)Get_ContextSettings_Struct(self))->settings.field =                     \
            (unsigned int)NUM2UINT(rb_value);                                                      \
        return rb_value;                                                                           \
    }

/* Macro-generated accessor pairs (depth_bits, stencil_bits,
 * antialiasing_level, major_version, minor_version); documented together via
 * Document-class's @!attribute tags below since the comment can't attach to
 * an individual macro expansion. */
CONTEXT_SETTINGS_UINT_ACCESSOR(depth_bits, depthBits)
CONTEXT_SETTINGS_UINT_ACCESSOR(stencil_bits, stencilBits)
CONTEXT_SETTINGS_UINT_ACCESSOR(antialiasing_level, antiAliasingLevel)
CONTEXT_SETTINGS_UINT_ACCESSOR(major_version, majorVersion)
CONTEXT_SETTINGS_UINT_ACCESSOR(minor_version, minorVersion)

/* Document-method: SFML::ContextSettings#attribute_flags
 * call-seq: attribute_flags -> Array<Symbol>
 *
 * @return [Array<Symbol>] a subset of +[:core, :debug]+, or +[:default]+ if
 *   neither is set
 */
static VALUE ContextSettings_get_attribute_flags(VALUE self) {
    uint32_t flags = ((ContextSettings*)Get_ContextSettings_Struct(self))->settings.attributeFlags;
    VALUE array = rb_ary_new();

    if ((flags & sfContextCore) != 0) {
        rb_ary_push(array, ID2SYM(rb_intern("core")));
    }

    if ((flags & sfContextDebug) != 0) {
        rb_ary_push(array, ID2SYM(rb_intern("debug")));
    }

    if (RARRAY_LEN(array) == 0) {
        rb_ary_push(array, ID2SYM(rb_intern("default")));
    }

    return array;
}

/* Document-method: SFML::ContextSettings#attribute_flags=
 * call-seq: attribute_flags = value -> value
 *
 * @return [Integer, Array<Symbol>] +value+
 */
static VALUE ContextSettings_set_attribute_flags(VALUE self, VALUE rb_value) {
    ContextSettings* settings = Get_ContextSettings_Struct(self);
    uint32_t flags = 0;

    rb_check_frozen(self);

    if (RB_INTEGER_TYPE_P(rb_value)) {
        flags = (uint32_t)NUM2UINT(rb_value);
    } else {
        VALUE array = RB_TYPE_P(rb_value, T_ARRAY) ? rb_value : rb_ary_new_from_args(1, rb_value);

        for (long i = 0; i < RARRAY_LEN(array); i++) {
            VALUE entry = rb_ary_entry(array, i);
            const char* name = context_flag_name(entry);

            if (strcmp(name, "core") == 0) {
                flags |= sfContextCore;
            } else if (strcmp(name, "debug") == 0) {
                flags |= sfContextDebug;
            }
        }
    }

    settings->settings.attributeFlags = flags;

    return rb_value;
}

/* call-seq: srgb_capable? -> true or false
 *
 * @return [Boolean]
 */
static VALUE ContextSettings_get_srgb(VALUE self) {
    return BOOL2RB(((ContextSettings*)Get_ContextSettings_Struct(self))->settings.sRgbCapable);
}

/* call-seq:
 *   srgb_capable=(value) -> true or false
 *
 * @return [Boolean] +value+
 */
static VALUE ContextSettings_set_srgb(VALUE self, VALUE rb_value) {
    rb_check_frozen(self);
    ((ContextSettings*)Get_ContextSettings_Struct(self))->settings.sRgbCapable = RTEST(rb_value);
    return rb_value;
}

/* Document-class: SFML::ContextSettings
 * Settings requested when creating a Window or Context: depth/stencil buffer
 * sizes, antialiasing level, requested OpenGL version, and attribute flags.
 *
 * The +depth_bits+/+stencil_bits+/+antialiasing_level+/+major_version+/
 * +minor_version+ accessors are generated through a shared C macro, so
 * (unlike this class's other accessors) they are documented here rather than
 * above their own definitions.
 *
 * @!attribute depth_bits
 *   @return [Integer] requested depth buffer bits per pixel
 * @!attribute stencil_bits
 *   @return [Integer] requested stencil buffer bits per pixel
 * @!attribute antialiasing_level
 *   @return [Integer] requested antialiasing level
 * @!attribute major_version
 *   @return [Integer] requested OpenGL major version
 * @!attribute minor_version
 *   @return [Integer] requested OpenGL minor version
 */
void Init_ContextSettings(VALUE rb_mSFML) {
    rb_cContextSettings = rb_define_class_under(rb_mSFML, "ContextSettings", rb_cObject);

    rb_define_singleton_method(rb_cContextSettings, "new", ContextSettings_new, -1);

    rb_define_method(rb_cContextSettings, "depth_bits", ContextSettings_get_depth_bits, 0);
    rb_define_method(rb_cContextSettings, "stencil_bits", ContextSettings_get_stencil_bits, 0);
    rb_define_method(rb_cContextSettings, "antialiasing_level",
                     ContextSettings_get_antialiasing_level, 0);
    rb_define_method(rb_cContextSettings, "major_version", ContextSettings_get_major_version, 0);
    rb_define_method(rb_cContextSettings, "minor_version", ContextSettings_get_minor_version, 0);
    rb_define_method(rb_cContextSettings, "attribute_flags", ContextSettings_get_attribute_flags,
                     0);
    rb_define_method(rb_cContextSettings, "srgb_capable?", ContextSettings_get_srgb, 0);

    rb_define_method(rb_cContextSettings, "depth_bits=", ContextSettings_set_depth_bits, 1);
    rb_define_method(rb_cContextSettings, "stencil_bits=", ContextSettings_set_stencil_bits, 1);
    rb_define_method(rb_cContextSettings,
                     "antialiasing_level=", ContextSettings_set_antialiasing_level, 1);
    rb_define_method(rb_cContextSettings, "major_version=", ContextSettings_set_major_version, 1);
    rb_define_method(rb_cContextSettings, "minor_version=", ContextSettings_set_minor_version, 1);
    rb_define_method(rb_cContextSettings, "attribute_flags=", ContextSettings_set_attribute_flags,
                     1);
    rb_define_method(rb_cContextSettings, "srgb_capable=", ContextSettings_set_srgb, 1);
}

VALUE Get_Klass_ContextSettings(void) {
    return rb_cContextSettings;
}

void* Get_ContextSettings_Struct(VALUE self) {
    ContextSettings* ptr;
    TypedData_Get_Struct(self, ContextSettings, &ContextSettings_data_type, ptr);
    return ptr;
}

sfContextSettings context_settings_from_rb(VALUE rb_settings) {
    if (NIL_P(rb_settings)) {
        return (sfContextSettings){0, 0, 0, 1, 1, sfContextDefault, false};
    }

    if (!rb_obj_is_kind_of(rb_settings, rb_cContextSettings)) {
        raise_invalid_argument_class(rb_cContextSettings);
    }

    return ((ContextSettings*)Get_ContextSettings_Struct(rb_settings))->settings;
}

VALUE context_settings_to_rb(sfContextSettings settings) {
    return ContextSettings_wrap(settings);
}
