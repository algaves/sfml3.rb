#include "system/input_stream.h"

#include <ruby.h>
#include <stdio.h>
#include <string.h>

#include "core/exceptions.h"

/* Wrap a Ruby IO-like object (read/seek/tell, optionally size). The struct is
   owned by the Ruby object; userData points at it, and dmark keeps the IO
   alive for as long as any load call might touch it. */
typedef struct {
    sfInputStream stream;
    VALUE rb_io;
} InputStream;

static VALUE rb_cInputStream;

static void InputStream_mark(void *ptr) {
    InputStream *stream = ptr;

    rb_gc_mark(stream->rb_io);
}

static void InputStream_free(void *ptr) {
    free(ptr);
}

static const rb_data_type_t InputStream_data_type = {
    .wrap_struct_name = "SFML::InputStream",
    .function = {.dmark = InputStream_mark, .dfree = InputStream_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

/* Callbacks run from C++ (SFML), so a Ruby exception must never unwind through
   those frames. Each body runs under rb_protect and reports failure as a
   negative result instead. */
typedef struct {
    VALUE io;
    size_t size;
    void *data;
    int64_t result;
} ReadContext;

static VALUE InputStream_read_body(VALUE v) {
    ReadContext *ctx = (ReadContext *) v;
    VALUE str = rb_funcall(ctx->io, rb_intern("read"), 1, SIZET2NUM(ctx->size));

    if (NIL_P(str)) {
        ctx->result = 0;
        return Qnil;
    }

    StringValue(str);

    long length = RSTRING_LEN(str);

    if ((size_t) length > ctx->size) {
        length = (long) ctx->size;
    }

    if (length > 0) {
        const unsigned char* src = (const unsigned char*)RSTRING_PTR(str);
        unsigned char* dest = ctx->data;
        long i;

        for (i = 0; i < length; i++) {
            dest[i] = src[i];
        }
    }

    ctx->result = (int64_t) length;

    return Qnil;
}

static int64_t InputStream_read(void *data, size_t size, void *userData) {
    InputStream *stream = userData;
    ReadContext ctx = {.io = stream->rb_io, .size = size, .data = data, .result = -1};
    int state = 0;

    rb_protect(InputStream_read_body, (VALUE) &ctx, &state);

    if (state) {
        rb_set_errinfo(Qnil);
        return -1;
    }

    return ctx.result;
}

typedef struct {
    VALUE io;
    size_t position;
    int64_t result;
} SeekContext;

static VALUE InputStream_seek_body(VALUE v) {
    SeekContext *ctx = (SeekContext *) v;

    rb_funcall(ctx->io, rb_intern("seek"), 2, SIZET2NUM(ctx->position), INT2NUM(SEEK_SET));
    ctx->result = (int64_t) ctx->position;

    return Qnil;
}

static int64_t InputStream_seek(size_t position, void *userData) {
    InputStream *stream = userData;
    SeekContext ctx = {.io = stream->rb_io, .position = position, .result = -1};
    int state = 0;

    rb_protect(InputStream_seek_body, (VALUE) &ctx, &state);

    if (state) {
        rb_set_errinfo(Qnil);
        return -1;
    }

    return ctx.result;
}

static VALUE InputStream_tell_body(VALUE v) {
    VALUE io = (VALUE) v;

    if (rb_respond_to(io, rb_intern("tell"))) {
        return rb_funcall(io, rb_intern("tell"), 0);
    }

    return rb_funcall(io, rb_intern("pos"), 0);
}

static int64_t InputStream_tell(void *userData) {
    InputStream *stream = userData;
    int state = 0;
    VALUE result = rb_protect(InputStream_tell_body, stream->rb_io, &state);

    if (state) {
        rb_set_errinfo(Qnil);
        return -1;
    }

    return NUM2LL(result);
}

static VALUE InputStream_get_size_body(VALUE v) {
    VALUE io = (VALUE) v;

    if (rb_respond_to(io, rb_intern("size"))) {
        return rb_funcall(io, rb_intern("size"), 0);
    }

    return rb_funcall(io, rb_intern("length"), 0);
}

static int64_t InputStream_get_size(void *userData) {
    InputStream *stream = userData;
    int state = 0;
    VALUE result = rb_protect(InputStream_get_size_body, stream->rb_io, &state);

    if (state) {
        rb_set_errinfo(Qnil);
        return -1;
    }

    return NUM2LL(result);
}

static VALUE InputStream_new(VALUE klass, VALUE rb_io) {
    VALUE self;
    InputStream *stream;

    if (!rb_respond_to(rb_io, rb_intern("read"))) {
        rb_raise(rb_eArgError, "stream object must respond to #read");
    }

    stream = malloc(sizeof(InputStream));

    stream->rb_io = rb_io;
    stream->stream.read = InputStream_read;
    stream->stream.seek = InputStream_seek;
    stream->stream.tell = InputStream_tell;
    stream->stream.getSize = InputStream_get_size;
    stream->stream.userData = stream;

    self = TypedData_Wrap_Struct(klass, &InputStream_data_type, stream);

    return self;
}

static VALUE InputStream_get_io(VALUE self) {
    return ((InputStream *) Get_InputStream_Struct(self))->rb_io;
}

void Init_InputStream(VALUE rb_module) {
    rb_cInputStream = rb_define_class_under(rb_module, "InputStream", rb_cObject);

    rb_define_singleton_method(rb_cInputStream, "new", InputStream_new, 1);

    rb_define_method(rb_cInputStream, "io", InputStream_get_io, 0);
}

VALUE Get_Klass_InputStream(void) {
    return rb_cInputStream;
}

sfInputStream *Get_InputStream_Struct(VALUE self) {
    InputStream *ptr;
    TypedData_Get_Struct(self, InputStream, &InputStream_data_type, ptr);
    return &ptr->stream;
}

int InputStream_is_stream(VALUE rb_stream) {
    return rb_obj_is_kind_of(rb_stream, rb_cInputStream) ? 1 : 0;
}

sfInputStream *input_stream_from_rb(VALUE rb_stream, VALUE *holder) {
    if (InputStream_is_stream(rb_stream)) {
        return Get_InputStream_Struct(rb_stream);
    }

    if (rb_respond_to(rb_stream, rb_intern("read"))) {
        VALUE wrapper = rb_funcall(rb_cInputStream, rb_intern("new"), 1, rb_stream);

        /* Callers keep this on their own stack so a GC triggered by the load
           call cannot collect the wrapper mid-flight. */
        if (holder != NULL) {
            *holder = wrapper;
        }

        return Get_InputStream_Struct(wrapper);
    }

    rb_raise(rb_eArgError, "expected an SFML::InputStream or an object responding to #read");
    return NULL;
}
