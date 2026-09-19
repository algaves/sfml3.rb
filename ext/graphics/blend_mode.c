#include "graphics/blend_mode.h"

#include <ruby.h>
#include <stdlib.h>
#include <string.h>

#include "core/exceptions.h"
#include "core/macros.h"

typedef struct {
    sfBlendMode mode;
} BlendMode;

static VALUE rb_cBlendMode;

static const char* factor_names[] = {"zero",      "one",
                                     "src_color", "one_minus_src_color",
                                     "dst_color", "one_minus_dst_color",
                                     "src_alpha", "one_minus_src_alpha",
                                     "dst_alpha", "one_minus_dst_alpha"};

static const char* equation_names[] = {"add", "subtract", "reverse_subtract", "min", "max"};

static void BlendMode_free(void* ptr) {
    free(ptr);
}

static const rb_data_type_t BlendMode_data_type = {
    .wrap_struct_name = "SFML::BlendMode",
    .function = {.dmark = NULL, .dfree = BlendMode_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE BlendMode_wrap(sfBlendMode mode) {
    BlendMode* ptr = malloc(sizeof(BlendMode));

    ptr->mode = mode;

    return TypedData_Wrap_Struct(rb_cBlendMode, &BlendMode_data_type, ptr);
}

static sfBlendFactor factor_from_rb(VALUE rb_factor) {
    size_t i;

    if (RB_INTEGER_TYPE_P(rb_factor)) {
        return (sfBlendFactor)NUM2INT(rb_factor);
    }

    if (SYMBOL_P(rb_factor)) {
        const char* name = rb_id2name(SYM2ID(rb_factor));

        for (i = 0; i < sizeof(factor_names) / sizeof(factor_names[0]); i++) {
            if (strcmp(name, factor_names[i]) == 0) {
                return (sfBlendFactor)i;
            }
        }
    }

    rb_raise(rb_eArgError, "unknown blend factor");
    return sfBlendFactorZero;
}

static sfBlendEquation equation_from_rb(VALUE rb_equation) {
    size_t i;

    if (RB_INTEGER_TYPE_P(rb_equation)) {
        return (sfBlendEquation)NUM2INT(rb_equation);
    }

    if (SYMBOL_P(rb_equation)) {
        const char* name = rb_id2name(SYM2ID(rb_equation));

        for (i = 0; i < sizeof(equation_names) / sizeof(equation_names[0]); i++) {
            if (strcmp(name, equation_names[i]) == 0) {
                return (sfBlendEquation)i;
            }
        }
    }

    rb_raise(rb_eArgError, "unknown blend equation");
    return sfBlendEquationAdd;
}

/* call-seq:
 *   BlendMode.new -> BlendMode(ALPHA)
 *   BlendMode.new(color_src_factor, color_dst_factor, color_equation, alpha_src_factor,
 * alpha_dst_factor, alpha_equation) -> BlendMode
 *
 * Each factor argument is a Symbol (one of +:zero+, +:one+, +:src_color+,
 * +:one_minus_src_color+, +:dst_color+, +:one_minus_dst_color+,
 * +:src_alpha+, +:one_minus_src_alpha+, +:dst_alpha+,
 * +:one_minus_dst_alpha+) or the equivalent Integer. Each equation argument
 * is a Symbol (+:add+, +:subtract+, +:reverse_subtract+, +:min+, +:max+) or
 * the equivalent Integer.
 *
 * @return [BlendMode]
 * @raise [ArgumentError] if given an argument count other than 0 or 6
 */
static VALUE BlendMode_new(int argc, VALUE* argv, VALUE klass) {
    VALUE self;
    BlendMode* ptr;
    sfBlendMode mode = sfBlendAlpha;

    if (argc == 6) {
        mode = (sfBlendMode){factor_from_rb(argv[0]),   factor_from_rb(argv[1]),
                             equation_from_rb(argv[2]), factor_from_rb(argv[3]),
                             factor_from_rb(argv[4]),   equation_from_rb(argv[5])};
    } else if (argc != 0) {
        raise_invalid_arguments_excepted(6, argc);
    }

    ptr = malloc(sizeof(BlendMode));
    ptr->mode = mode;

    self = TypedData_Wrap_Struct(klass, &BlendMode_data_type, ptr);

    return self;
}

/* call-seq: color_src_factor -> Symbol
 *
 * @return [Symbol] the source blending factor for the color/RGB channels
 */
static VALUE BlendMode_get_color_src_factor(VALUE self) {
    return ID2SYM(
        rb_intern(factor_names[((BlendMode*)Get_BlendMode_Struct(self))->mode.colorSrcFactor]));
}

/* call-seq: color_dst_factor -> Symbol
 *
 * @return [Symbol] the destination blending factor for the color/RGB channels
 */
static VALUE BlendMode_get_color_dst_factor(VALUE self) {
    return ID2SYM(
        rb_intern(factor_names[((BlendMode*)Get_BlendMode_Struct(self))->mode.colorDstFactor]));
}

/* call-seq: color_equation -> Symbol
 *
 * @return [Symbol] the blending equation for the color/RGB channels
 */
static VALUE BlendMode_get_color_equation(VALUE self) {
    return ID2SYM(
        rb_intern(equation_names[((BlendMode*)Get_BlendMode_Struct(self))->mode.colorEquation]));
}

/* call-seq: alpha_src_factor -> Symbol
 *
 * @return [Symbol] the source blending factor for the alpha channel
 */
static VALUE BlendMode_get_alpha_src_factor(VALUE self) {
    return ID2SYM(
        rb_intern(factor_names[((BlendMode*)Get_BlendMode_Struct(self))->mode.alphaSrcFactor]));
}

/* call-seq: alpha_dst_factor -> Symbol
 *
 * @return [Symbol] the destination blending factor for the alpha channel
 */
static VALUE BlendMode_get_alpha_dst_factor(VALUE self) {
    return ID2SYM(
        rb_intern(factor_names[((BlendMode*)Get_BlendMode_Struct(self))->mode.alphaDstFactor]));
}

/* call-seq: alpha_equation -> Symbol
 *
 * @return [Symbol] the blending equation for the alpha channel
 */
static VALUE BlendMode_get_alpha_equation(VALUE self) {
    return ID2SYM(
        rb_intern(equation_names[((BlendMode*)Get_BlendMode_Struct(self))->mode.alphaEquation]));
}

/* call-seq:
 *   color_src_factor=(value) -> Symbol
 *
 * @return [Symbol] +value+
 */
static VALUE BlendMode_set_color_src_factor(VALUE self, VALUE rb_value) {
    rb_check_frozen(self);
    ((BlendMode*)Get_BlendMode_Struct(self))->mode.colorSrcFactor = factor_from_rb(rb_value);
    return rb_value;
}

/* call-seq:
 *   color_dst_factor=(value) -> Symbol
 *
 * @return [Symbol] +value+
 */
static VALUE BlendMode_set_color_dst_factor(VALUE self, VALUE rb_value) {
    rb_check_frozen(self);
    ((BlendMode*)Get_BlendMode_Struct(self))->mode.colorDstFactor = factor_from_rb(rb_value);
    return rb_value;
}

/* call-seq:
 *   color_equation=(value) -> Symbol
 *
 * @return [Symbol] +value+
 */
static VALUE BlendMode_set_color_equation(VALUE self, VALUE rb_value) {
    rb_check_frozen(self);
    ((BlendMode*)Get_BlendMode_Struct(self))->mode.colorEquation = equation_from_rb(rb_value);
    return rb_value;
}

/* call-seq:
 *   alpha_src_factor=(value) -> Symbol
 *
 * @return [Symbol] +value+
 */
static VALUE BlendMode_set_alpha_src_factor(VALUE self, VALUE rb_value) {
    rb_check_frozen(self);
    ((BlendMode*)Get_BlendMode_Struct(self))->mode.alphaSrcFactor = factor_from_rb(rb_value);
    return rb_value;
}

/* call-seq:
 *   alpha_dst_factor=(value) -> Symbol
 *
 * @return [Symbol] +value+
 */
static VALUE BlendMode_set_alpha_dst_factor(VALUE self, VALUE rb_value) {
    rb_check_frozen(self);
    ((BlendMode*)Get_BlendMode_Struct(self))->mode.alphaDstFactor = factor_from_rb(rb_value);
    return rb_value;
}

/* call-seq:
 *   alpha_equation=(value) -> Symbol
 *
 * @return [Symbol] +value+
 */
static VALUE BlendMode_set_alpha_equation(VALUE self, VALUE rb_value) {
    rb_check_frozen(self);
    ((BlendMode*)Get_BlendMode_Struct(self))->mode.alphaEquation = equation_from_rb(rb_value);
    return rb_value;
}

/* call-seq:
 *   self == other -> true or false
 *
 * @return [Boolean]
 */
static VALUE BlendMode_eql(VALUE self, VALUE rb_other) {
    sfBlendMode a = ((BlendMode*)Get_BlendMode_Struct(self))->mode;
    sfBlendMode b;

    if (!rb_obj_is_kind_of(rb_other, rb_cBlendMode)) {
        return Qfalse;
    }

    b = ((BlendMode*)Get_BlendMode_Struct(rb_other))->mode;

    return BOOL2RB(a.colorSrcFactor == b.colorSrcFactor && a.colorDstFactor == b.colorDstFactor &&
                   a.colorEquation == b.colorEquation && a.alphaSrcFactor == b.alphaSrcFactor &&
                   a.alphaDstFactor == b.alphaDstFactor && a.alphaEquation == b.alphaEquation);
}

/* Document-class: SFML::BlendMode
 * Describes how the colors of a drawn object are combined with the colors
 * already in the render target, via two triples of (source factor,
 * destination factor, equation) -- one for the color/RGB channels, one for
 * the alpha channel.
 *
 * @!attribute color_src_factor
 *   @return [Symbol] the source blending factor for the color channels
 * @!attribute color_dst_factor
 *   @return [Symbol] the destination blending factor for the color channels
 * @!attribute color_equation
 *   @return [Symbol] the blending equation for the color channels
 * @!attribute alpha_src_factor
 *   @return [Symbol] the source blending factor for the alpha channel
 * @!attribute alpha_dst_factor
 *   @return [Symbol] the destination blending factor for the alpha channel
 * @!attribute alpha_equation
 *   @return [Symbol] the blending equation for the alpha channel
 */
void Init_BlendMode(VALUE rb_mSFML) {
    rb_cBlendMode = rb_define_class_under(rb_mSFML, "BlendMode", rb_cObject);

    rb_define_singleton_method(rb_cBlendMode, "new", BlendMode_new, -1);

    rb_define_method(rb_cBlendMode, "color_src_factor", BlendMode_get_color_src_factor, 0);
    rb_define_method(rb_cBlendMode, "color_dst_factor", BlendMode_get_color_dst_factor, 0);
    rb_define_method(rb_cBlendMode, "color_equation", BlendMode_get_color_equation, 0);
    rb_define_method(rb_cBlendMode, "alpha_src_factor", BlendMode_get_alpha_src_factor, 0);
    rb_define_method(rb_cBlendMode, "alpha_dst_factor", BlendMode_get_alpha_dst_factor, 0);
    rb_define_method(rb_cBlendMode, "alpha_equation", BlendMode_get_alpha_equation, 0);

    rb_define_method(rb_cBlendMode, "color_src_factor=", BlendMode_set_color_src_factor, 1);
    rb_define_method(rb_cBlendMode, "color_dst_factor=", BlendMode_set_color_dst_factor, 1);
    rb_define_method(rb_cBlendMode, "color_equation=", BlendMode_set_color_equation, 1);
    rb_define_method(rb_cBlendMode, "alpha_src_factor=", BlendMode_set_alpha_src_factor, 1);
    rb_define_method(rb_cBlendMode, "alpha_dst_factor=", BlendMode_set_alpha_dst_factor, 1);
    rb_define_method(rb_cBlendMode, "alpha_equation=", BlendMode_set_alpha_equation, 1);

    rb_define_method(rb_cBlendMode, "==", BlendMode_eql, 1);

    /* No blending: source pixels overwrite destination pixels unchanged
     * (One, Zero, Add for both the color and alpha channels). */
    rb_define_const(rb_cBlendMode, "NONE", rb_obj_freeze(BlendMode_wrap(sfBlendNone)));

    /* Standard alpha blending: the source is blended over the destination
     * using the source alpha (SrcAlpha, OneMinusSrcAlpha, Add for color;
     * One, OneMinusSrcAlpha, Add for alpha). The default blend mode. */
    rb_define_const(rb_cBlendMode, "ALPHA", rb_obj_freeze(BlendMode_wrap(sfBlendAlpha)));

    /* Additive blending: the source color, scaled by its alpha, is added to
     * the destination (SrcAlpha, One, Add for color; One, One, Add for
     * alpha). */
    rb_define_const(rb_cBlendMode, "ADD", rb_obj_freeze(BlendMode_wrap(sfBlendAdd)));

    /* Multiplicative blending: source and destination colors are multiplied
     * together (DstColor, Zero, Add for both the color and alpha
     * channels). */
    rb_define_const(rb_cBlendMode, "MULTIPLY", rb_obj_freeze(BlendMode_wrap(sfBlendMultiply)));

    /* Component-wise minimum of the source and destination colors (One,
     * One, Min for both the color and alpha channels). */
    rb_define_const(rb_cBlendMode, "MIN", rb_obj_freeze(BlendMode_wrap(sfBlendMin)));

    /* Component-wise maximum of the source and destination colors (One,
     * One, Max for both the color and alpha channels). */
    rb_define_const(rb_cBlendMode, "MAX", rb_obj_freeze(BlendMode_wrap(sfBlendMax)));
}

VALUE Get_Klass_BlendMode(void) {
    return rb_cBlendMode;
}

void* Get_BlendMode_Struct(VALUE self) {
    BlendMode* ptr;
    TypedData_Get_Struct(self, BlendMode, &BlendMode_data_type, ptr);
    return ptr;
}

sfBlendMode blend_mode_from_rb(VALUE rb_mode) {
    if (rb_obj_is_kind_of(rb_mode, rb_cBlendMode)) {
        return ((BlendMode*)Get_BlendMode_Struct(rb_mode))->mode;
    }

    if (RB_INTEGER_TYPE_P(rb_mode) == 0 && !SYMBOL_P(rb_mode)) {
        raise_invalid_argument_class(rb_cBlendMode);
    }

    if (SYMBOL_P(rb_mode)) {
        const char* name = rb_id2name(SYM2ID(rb_mode));

        if (strcmp(name, "none") == 0) {
            return sfBlendNone;
        }

        if (strcmp(name, "alpha") == 0) {
            return sfBlendAlpha;
        }

        if (strcmp(name, "add") == 0) {
            return sfBlendAdd;
        }

        if (strcmp(name, "multiply") == 0) {
            return sfBlendMultiply;
        }

        if (strcmp(name, "min") == 0) {
            return sfBlendMin;
        }

        if (strcmp(name, "max") == 0) {
            return sfBlendMax;
        }
    }

    return sfBlendAlpha;
}

VALUE blend_mode_to_rb(sfBlendMode mode) {
    return BlendMode_wrap(mode);
}
