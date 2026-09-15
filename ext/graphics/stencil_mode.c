#include "graphics/stencil_mode.h"

#include <ruby.h>
#include <stdlib.h>
#include <string.h>

#include "core/exceptions.h"
#include "core/macros.h"

typedef struct {
    sfStencilMode mode;
} StencilMode;

static VALUE rb_cStencilMode;

static const char *comparison_names[] = {
    "never", "less", "less_equal", "greater", "greater_equal", "equal", "not_equal", "always"
};

static const char *operation_names[] = {
    "keep", "zero", "replace", "increment", "decrement", "invert"
};

static void StencilMode_free(void *ptr) {
    free(ptr);
}

static const rb_data_type_t StencilMode_data_type = {
    .wrap_struct_name = "SFML::StencilMode",
    .function = {.dmark = NULL, .dfree = StencilMode_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE StencilMode_wrap(sfStencilMode mode) {
    StencilMode *ptr = malloc(sizeof(StencilMode));

    ptr->mode = mode;

    return TypedData_Wrap_Struct(rb_cStencilMode, &StencilMode_data_type, ptr);
}

static sfStencilComparison comparison_from_rb(VALUE rb_value) {
    size_t i;

    if (RB_INTEGER_TYPE_P(rb_value)) {
        return (sfStencilComparison) NUM2INT(rb_value);
    }

    if (SYMBOL_P(rb_value)) {
        const char *name = rb_id2name(SYM2ID(rb_value));

        for (i = 0; i < sizeof(comparison_names) / sizeof(comparison_names[0]); i++) {
            if (strcmp(name, comparison_names[i]) == 0) {
                return (sfStencilComparison) i;
            }
        }
    }

    rb_raise(rb_eArgError, "unknown stencil comparison");
    return sfStencilComparisonAlways;
}

static sfStencilUpdateOperation operation_from_rb(VALUE rb_value) {
    size_t i;

    if (RB_INTEGER_TYPE_P(rb_value)) {
        return (sfStencilUpdateOperation) NUM2INT(rb_value);
    }

    if (SYMBOL_P(rb_value)) {
        const char *name = rb_id2name(SYM2ID(rb_value));

        for (i = 0; i < sizeof(operation_names) / sizeof(operation_names[0]); i++) {
            if (strcmp(name, operation_names[i]) == 0) {
                return (sfStencilUpdateOperation) i;
            }
        }
    }

    rb_raise(rb_eArgError, "unknown stencil update operation");
    return sfStencilUpdateOperationKeep;
}

static VALUE StencilMode_new(int argc, VALUE *argv, VALUE klass) {
    VALUE self;
    StencilMode *ptr;
    sfStencilMode mode = sfStencilMode_default;

    if (argc == 5) {
        mode.stencilComparison = comparison_from_rb(argv[0]);
        mode.stencilUpdateOperation = operation_from_rb(argv[1]);
        mode.stencilReference.value = (unsigned int) NUM2UINT(argv[2]);
        mode.stencilMask.value = (unsigned int) NUM2UINT(argv[3]);
        mode.stencilOnly = RTEST(argv[4]);
    } else if (argc != 0) {
        raise_invalid_arguments_excepted(5, argc);
    }

    ptr = malloc(sizeof(StencilMode));
    ptr->mode = mode;

    self = TypedData_Wrap_Struct(klass, &StencilMode_data_type, ptr);

    return self;
}

static VALUE StencilMode_get_comparison(VALUE self) {
    return ID2SYM(rb_intern(comparison_names[((StencilMode *) Get_StencilMode_Struct(self))->mode.stencilComparison]));
}

static VALUE StencilMode_get_operation(VALUE self) {
    sfStencilMode mode = ((StencilMode *) Get_StencilMode_Struct(self))->mode;

    return ID2SYM(rb_intern(operation_names[mode.stencilUpdateOperation]));
}

static VALUE StencilMode_get_reference(VALUE self) {
    return UINT2NUM(((StencilMode *) Get_StencilMode_Struct(self))->mode.stencilReference.value);
}

static VALUE StencilMode_get_mask(VALUE self) {
    return UINT2NUM(((StencilMode *) Get_StencilMode_Struct(self))->mode.stencilMask.value);
}

static VALUE StencilMode_get_only(VALUE self) {
    return BOOL2RB(((StencilMode *) Get_StencilMode_Struct(self))->mode.stencilOnly);
}

static VALUE StencilMode_set_comparison(VALUE self, VALUE rb_value) {
    ((StencilMode *) Get_StencilMode_Struct(self))->mode.stencilComparison = comparison_from_rb(rb_value);
    return rb_value;
}

static VALUE StencilMode_set_operation(VALUE self, VALUE rb_value) {
    ((StencilMode *) Get_StencilMode_Struct(self))->mode.stencilUpdateOperation = operation_from_rb(rb_value);
    return rb_value;
}

static VALUE StencilMode_set_reference(VALUE self, VALUE rb_value) {
    ((StencilMode *) Get_StencilMode_Struct(self))->mode.stencilReference.value = (unsigned int) NUM2UINT(rb_value);
    return rb_value;
}

static VALUE StencilMode_set_mask(VALUE self, VALUE rb_value) {
    ((StencilMode *) Get_StencilMode_Struct(self))->mode.stencilMask.value = (unsigned int) NUM2UINT(rb_value);
    return rb_value;
}

static VALUE StencilMode_set_only(VALUE self, VALUE rb_value) {
    ((StencilMode *) Get_StencilMode_Struct(self))->mode.stencilOnly = RTEST(rb_value);
    return rb_value;
}

static VALUE StencilMode_eql(VALUE self, VALUE rb_other) {
    sfStencilMode a = ((StencilMode *) Get_StencilMode_Struct(self))->mode;
    sfStencilMode b;

    if (!rb_obj_is_kind_of(rb_other, rb_cStencilMode)) {
        return Qfalse;
    }

    b = ((StencilMode *) Get_StencilMode_Struct(rb_other))->mode;

    return BOOL2RB(a.stencilComparison == b.stencilComparison &&
                   a.stencilUpdateOperation == b.stencilUpdateOperation &&
                   a.stencilReference.value == b.stencilReference.value &&
                   a.stencilMask.value == b.stencilMask.value &&
                   a.stencilOnly == b.stencilOnly);
}

void Init_StencilMode(VALUE rb_module) {
    rb_cStencilMode = rb_define_class_under(rb_module, "StencilMode", rb_cObject);

    rb_define_singleton_method(rb_cStencilMode, "new", StencilMode_new, -1);

    rb_define_method(rb_cStencilMode, "comparison", StencilMode_get_comparison, 0);
    rb_define_method(rb_cStencilMode, "update_operation", StencilMode_get_operation, 0);
    rb_define_method(rb_cStencilMode, "reference", StencilMode_get_reference, 0);
    rb_define_method(rb_cStencilMode, "mask", StencilMode_get_mask, 0);
    rb_define_method(rb_cStencilMode, "stencil_only", StencilMode_get_only, 0);

    rb_define_method(rb_cStencilMode, "comparison=", StencilMode_set_comparison, 1);
    rb_define_method(rb_cStencilMode, "update_operation=", StencilMode_set_operation, 1);
    rb_define_method(rb_cStencilMode, "reference=", StencilMode_set_reference, 1);
    rb_define_method(rb_cStencilMode, "mask=", StencilMode_set_mask, 1);
    rb_define_method(rb_cStencilMode, "stencil_only=", StencilMode_set_only, 1);

    rb_define_method(rb_cStencilMode, "==", StencilMode_eql, 1);
}

VALUE Get_Klass_StencilMode(void) {
    return rb_cStencilMode;
}

void *Get_StencilMode_Struct(VALUE self) {
    StencilMode *ptr;
    TypedData_Get_Struct(self, StencilMode, &StencilMode_data_type, ptr);
    return ptr;
}

sfStencilMode stencil_mode_from_rb(VALUE rb_mode) {
    if (rb_obj_is_kind_of(rb_mode, rb_cStencilMode)) {
        return ((StencilMode *) Get_StencilMode_Struct(rb_mode))->mode;
    }

    return sfStencilMode_default;
}

VALUE stencil_mode_to_rb(sfStencilMode mode) {
    return StencilMode_wrap(mode);
}
