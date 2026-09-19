#include "graphics/rect.h"

#include <stdio.h>
#include <stdlib.h>

#include "core/exceptions.h"
#include "core/macros.h"
#include "system/vec2.h"

typedef struct {
    sfFloatRect rect;
} Rect;

static VALUE rb_cRect;

static void Rect_free(void* ptr) {
    free(ptr);
}

static const rb_data_type_t Rect_data_type = {
    .wrap_struct_name = "SFML::Rect",
    .function = {.dmark = NULL, .dfree = Rect_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE Rect_wrap(sfFloatRect rect) {
    Rect* ptr = malloc(sizeof(Rect));

    ptr->rect = rect;

    return TypedData_Wrap_Struct(rb_cRect, &Rect_data_type, ptr);
}

static VALUE Rect_alloc(VALUE klass) {
    Rect* ptr = malloc(sizeof(Rect));

    if (ptr == NULL) {
        rb_raise(rb_eNoMemError, "failed to allocate rect");
    }

    ptr->rect = (sfFloatRect){{0, 0}, {0, 0}};

    return TypedData_Wrap_Struct(klass, &Rect_data_type, ptr);
}

/* call-seq:
 *   Rect.new                        -> Rect(0, 0, 0, 0)
 *   Rect.new(left, top)             -> Rect(left, top, 0, 0)
 *   Rect.new(left, top, width, height) -> Rect(left, top, width, height)
 *   Rect.new([left, top, width, height]) -> Rect(left, top, width, height)
 *   Rect.new(other_rect)            -> copy of +other_rect+
 *
 * Used for both float rects (positions/sizes) and int rects (pixel
 * sub-regions of a texture); the underlying storage is always Float.
 *
 * @return [Rect]
 * @raise [ArgumentError] if given an Array shorter than 4 elements, or an
 *   argument count other than 0, 1, 2 or 4
 */
static VALUE Rect_initialize(int argc, VALUE* argv, VALUE self) {
    sfFloatRect rect = {{0, 0}, {0, 0}};

    if (argc == 1 && rb_obj_is_kind_of(argv[0], rb_cRect)) {
        rect = ((Rect*)Get_Rect_Struct(argv[0]))->rect;
    } else if (argc == 1 && RB_TYPE_P(argv[0], T_ARRAY)) {
        rect = rect_from_rb(argv[0]);
    } else if (argc == 2) {
        rect = (sfFloatRect){{(float)NUM2DBL(argv[0]), (float)NUM2DBL(argv[1])}, {0, 0}};
    } else if (argc == 4) {
        rect = (sfFloatRect){{(float)NUM2DBL(argv[0]), (float)NUM2DBL(argv[1])},
                             {(float)NUM2DBL(argv[2]), (float)NUM2DBL(argv[3])}};
    } else if (argc != 0) {
        raise_invalid_arguments_excepted(4, argc);
    }

    ((Rect*)Get_Rect_Struct(self))->rect = rect;

    return self;
}

/* call-seq: left -> Float
 *
 * Returns the X coordinate of the left edge.
 *
 * @return [Float]
 */
static VALUE Rect_get_left(VALUE self) {
    return DBL2NUM(((Rect*)Get_Rect_Struct(self))->rect.position.x);
}

/* call-seq: top -> Float
 *
 * Returns the Y coordinate of the top edge.
 *
 * @return [Float]
 */
static VALUE Rect_get_top(VALUE self) {
    return DBL2NUM(((Rect*)Get_Rect_Struct(self))->rect.position.y);
}

/* call-seq: width -> Float
 *
 * Returns the width.
 *
 * @return [Float]
 */
static VALUE Rect_get_width(VALUE self) {
    return DBL2NUM(((Rect*)Get_Rect_Struct(self))->rect.size.x);
}

/* call-seq: height -> Float
 *
 * Returns the height.
 *
 * @return [Float]
 */
static VALUE Rect_get_height(VALUE self) {
    return DBL2NUM(((Rect*)Get_Rect_Struct(self))->rect.size.y);
}

/* call-seq:
 *   left=(value) -> Float
 *
 * Sets the X coordinate of the left edge.
 *
 * @return [Float] +value+
 */
static VALUE Rect_set_left(VALUE self, VALUE rb_value) {
    ((Rect*)Get_Rect_Struct(self))->rect.position.x = NUM2DBL(rb_value);
    return rb_value;
}

/* call-seq:
 *   top=(value) -> Float
 *
 * Sets the Y coordinate of the top edge.
 *
 * @return [Float] +value+
 */
static VALUE Rect_set_top(VALUE self, VALUE rb_value) {
    ((Rect*)Get_Rect_Struct(self))->rect.position.y = NUM2DBL(rb_value);
    return rb_value;
}

/* call-seq:
 *   width=(value) -> Float
 *
 * Sets the width.
 *
 * @return [Float] +value+
 */
static VALUE Rect_set_width(VALUE self, VALUE rb_value) {
    ((Rect*)Get_Rect_Struct(self))->rect.size.x = NUM2DBL(rb_value);
    return rb_value;
}

/* call-seq:
 *   height=(value) -> Float
 *
 * Sets the height.
 *
 * @return [Float] +value+
 */
static VALUE Rect_set_height(VALUE self, VALUE rb_value) {
    ((Rect*)Get_Rect_Struct(self))->rect.size.y = NUM2DBL(rb_value);
    return rb_value;
}

/* call-seq: position -> Vector2
 *
 * Returns the object's position.
 *
 * @return [Vector2] the +(left, top)+ corner
 */
static VALUE Rect_get_position(VALUE self) {
    sfFloatRect rect = ((Rect*)Get_Rect_Struct(self))->rect;

    return vec2f_to_rb(rect.position);
}

/* call-seq: size -> Vector2
 *
 * Returns the object's size.
 *
 * @return [Vector2] the +(width, height)+ dimensions
 */
static VALUE Rect_get_size(VALUE self) {
    sfFloatRect rect = ((Rect*)Get_Rect_Struct(self))->rect;

    return vec2f_to_rb(rect.size);
}

/* call-seq:
 *   position=(value) -> Vector2
 *
 * Sets the object's position.
 *
 * @return [Vector2] +value+
 */
static VALUE Rect_set_position(VALUE self, VALUE rb_position) {
    ((Rect*)Get_Rect_Struct(self))->rect.position = vec2f_from_rb(rb_position);
    return rb_position;
}

/* call-seq:
 *   size=(value) -> Vector2
 *
 * Sets the object's size.
 *
 * @return [Vector2] +value+
 */
static VALUE Rect_set_size(VALUE self, VALUE rb_size) {
    ((Rect*)Get_Rect_Struct(self))->rect.size = vec2f_from_rb(rb_size);
    return rb_size;
}

/* call-seq:
 *   contains?(point) -> true or false
 *   contains?(x, y) -> true or false
 *
 * Returns +true+ if the rectangle contains the given point.
 *
 * @return [Boolean]
 * @raise [ArgumentError] if given an argument count other than 1 or 2
 */
static VALUE Rect_contains(int argc, VALUE* argv, VALUE self) {
    sfFloatRect rect = ((Rect*)Get_Rect_Struct(self))->rect;
    sfVector2f point;

    if (argc == 1) {
        point = vec2f_from_rb(argv[0]);
    } else if (argc == 2) {
        point = (sfVector2f){(float)NUM2DBL(argv[0]), (float)NUM2DBL(argv[1])};
    } else {
        raise_invalid_arguments_excepted(2, argc);
        return Qfalse;
    }

    return BOOL2RB(sfFloatRect_contains(&rect, point));
}

/* call-seq:
 *   intersection(other) -> Rect or nil
 *
 * Returns the overlapping region of two rectangles.
 *
 * @return [Rect, nil] the overlapping region, or +nil+ if the rects don't
 *   intersect
 */
static VALUE Rect_intersection(VALUE self, VALUE rb_other) {
    sfFloatRect a = ((Rect*)Get_Rect_Struct(self))->rect;
    sfFloatRect b = rect_from_rb(rb_other);
    sfFloatRect intersection;

    if (sfFloatRect_intersects(&a, &b, &intersection)) {
        return Rect_wrap(intersection);
    }

    return Qnil;
}

/* call-seq:
 *   intersects?(other) -> true or false
 *
 * Returns +true+ if the two rectangles overlap.
 *
 * @return [Boolean]
 */
static VALUE Rect_intersects(VALUE self, VALUE rb_other) {
    return BOOL2RB(Rect_intersection(self, rb_other) != Qnil);
}

/* call-seq: to_a -> [Float, Float, Float, Float]
 *
 * Returns the object as an Array.
 *
 * @return [Array<Float>] +[left, top, width, height]+
 */
static VALUE Rect_to_a(VALUE self) {
    sfFloatRect rect = ((Rect*)Get_Rect_Struct(self))->rect;

    return rb_ary_new_from_args(4, DBL2NUM(rect.position.x), DBL2NUM(rect.position.y),
                                DBL2NUM(rect.size.x), DBL2NUM(rect.size.y));
}

/* call-seq: each { |component| ... } -> self
 *
 * Yields +left+, +top+, +width+, +height+ in that order.
 *
 * @return [self]
 */
static VALUE Rect_each(VALUE self) {
    VALUE array = Rect_to_a(self);
    long i;

    for (i = 0; i < 4; i++) {
        rb_yield(rb_ary_entry(array, i));
    }

    return self;
}

/* call-seq:
 *   self == other -> true or false
 *
 * +other+ may be a Rect or a 4-element Array.
 *
 * @return [Boolean]
 */
static VALUE Rect_eql(VALUE self, VALUE rb_other) {
    sfFloatRect a = ((Rect*)Get_Rect_Struct(self))->rect;
    sfFloatRect b;

    if (rb_obj_is_kind_of(rb_other, rb_cRect)) {
        b = ((Rect*)Get_Rect_Struct(rb_other))->rect;
    } else if (RB_TYPE_P(rb_other, T_ARRAY) && RARRAY_LEN(rb_other) >= 4) {
        b = rect_from_rb(rb_other);
    } else {
        return Qfalse;
    }

    return BOOL2RB(a.position.x == b.position.x && a.position.y == b.position.y &&
                   a.size.x == b.size.x && a.size.y == b.size.y);
}

/* call-seq: to_s -> String
 *
 * Returns a human-readable representation of the object.
 *
 * @return [String] +"(left, top, width, height)"+
 */
static VALUE Rect_to_s(VALUE self) {
    sfFloatRect rect = ((Rect*)Get_Rect_Struct(self))->rect;
    char buffer[96];

    snprintf(buffer, sizeof(buffer), "(%g, %g, %g, %g)", rect.position.x, rect.position.y,
             rect.size.x, rect.size.y);

    return rb_str_new2(buffer);
}

/* Document-class: SFML::Rect
 * An axis-aligned rectangle, stored as a position +(left, top)+ and a size
 * +(width, height)+. Used both for float rects (bounds, viewports) and int
 * rects (texture sub-regions) -- the underlying storage is always Float.
 *
 * Includes +Enumerable+ and behaves like a 4-element sequence.
 *
 * @!attribute left
 *   The X coordinate of the left edge.
 *   @return [Float] the X coordinate of the top-left corner
 * @!attribute top
 *   The Y coordinate of the top edge.
 *   @return [Float] the Y coordinate of the top-left corner
 * @!attribute width
 *   The width.
 *   @return [Float] the width
 * @!attribute height
 *   The height.
 *   @return [Float] the height
 * @!attribute position
 *   The object's position.
 *   @return [Vector2] the +(left, top)+ corner
 * @!attribute size
 *   The object's size.
 *   @return [Vector2] the +(width, height)+ dimensions
 */
void Init_Rect(VALUE rb_mSFML) {
    rb_cRect = rb_define_class_under(rb_mSFML, "Rect", rb_cObject);

    rb_include_module(rb_cRect, rb_mEnumerable);

    rb_define_alloc_func(rb_cRect, Rect_alloc);
    rb_define_method(rb_cRect, "initialize", Rect_initialize, -1);

    rb_define_method(rb_cRect, "left", Rect_get_left, 0);
    rb_define_method(rb_cRect, "top", Rect_get_top, 0);
    rb_define_method(rb_cRect, "width", Rect_get_width, 0);
    rb_define_method(rb_cRect, "height", Rect_get_height, 0);
    rb_define_method(rb_cRect, "left=", Rect_set_left, 1);
    rb_define_method(rb_cRect, "top=", Rect_set_top, 1);
    rb_define_method(rb_cRect, "width=", Rect_set_width, 1);
    rb_define_method(rb_cRect, "height=", Rect_set_height, 1);

    rb_define_method(rb_cRect, "position", Rect_get_position, 0);
    rb_define_method(rb_cRect, "size", Rect_get_size, 0);
    rb_define_method(rb_cRect, "position=", Rect_set_position, 1);
    rb_define_method(rb_cRect, "size=", Rect_set_size, 1);

    rb_define_method(rb_cRect, "contains?", Rect_contains, -1);
    rb_define_method(rb_cRect, "intersects?", Rect_intersects, 1);
    rb_define_method(rb_cRect, "intersection", Rect_intersection, 1);

    rb_define_method(rb_cRect, "to_a", Rect_to_a, 0);
    rb_define_method(rb_cRect, "to_ary", Rect_to_a, 0);
    rb_define_method(rb_cRect, "each", Rect_each, 0);
    rb_define_method(rb_cRect, "==", Rect_eql, 1);
    rb_define_method(rb_cRect, "to_s", Rect_to_s, 0);
}

VALUE Get_Klass_Rect(void) {
    return rb_cRect;
}

void* Get_Rect_Struct(VALUE self) {
    Rect* ptr;
    TypedData_Get_Struct(self, Rect, &Rect_data_type, ptr);
    return ptr;
}

sfFloatRect rect_from_rb(VALUE rb_rect) {
    if (rb_obj_is_kind_of(rb_rect, rb_cRect)) {
        return ((Rect*)Get_Rect_Struct(rb_rect))->rect;
    }

    if (RB_TYPE_P(rb_rect, T_ARRAY)) {
        if (RARRAY_LEN(rb_rect) < 4) {
            raise_invalid_array_length(4);
        }

        return (sfFloatRect){
            {(float)NUM2DBL(rb_ary_entry(rb_rect, 0)), (float)NUM2DBL(rb_ary_entry(rb_rect, 1))},
            {(float)NUM2DBL(rb_ary_entry(rb_rect, 2)), (float)NUM2DBL(rb_ary_entry(rb_rect, 3))}};
    }

    raise_invalid_argument_class(rb_cRect);

    return (sfFloatRect){{0, 0}, {0, 0}};
}

VALUE rect_to_rb(sfFloatRect c_rect) {
    return Rect_wrap(c_rect);
}

sfIntRect int_rect_from_rb(VALUE rb_rect) {
    sfFloatRect rect = rect_from_rb(rb_rect);

    return (sfIntRect){{(int)rect.position.x, (int)rect.position.y},
                       {(int)rect.size.x, (int)rect.size.y}};
}

VALUE int_rect_to_rb(sfIntRect c_rect) {
    return Rect_wrap((sfFloatRect){{(float)c_rect.position.x, (float)c_rect.position.y},
                                   {(float)c_rect.size.x, (float)c_rect.size.y}});
}

void Rect_check(VALUE rb_rect) {
    if (rb_obj_is_kind_of(rb_rect, rb_cRect) || RB_TYPE_P(rb_rect, T_ARRAY)) {
        return;
    }

    raise_invalid_argument_class(rb_cRect);
}
