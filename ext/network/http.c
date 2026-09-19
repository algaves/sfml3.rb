#include "network/http.h"

#include <ruby.h>
#include <stdlib.h>

#include "core/macros.h"
#include "network/network_enums.h"
#include "system/time.h"

typedef struct {
    sfHttp* handle;
} Http;

typedef struct {
    sfHttpRequest* handle;
} HttpRequest;

typedef struct {
    sfHttpResponse* handle;
} HttpResponse;

static VALUE rb_cHttp;
static VALUE rb_cHttpRequest;
static VALUE rb_cHttpResponse;

static void Http_free(void* ptr) {
    Http* http = ptr;

    sfHttp_destroy(http->handle);
    free(http);
}

static void HttpRequest_free(void* ptr) {
    HttpRequest* request = ptr;

    sfHttpRequest_destroy(request->handle);
    free(request);
}

static void HttpResponse_free(void* ptr) {
    HttpResponse* response = ptr;

    sfHttpResponse_destroy(response->handle);
    free(response);
}

static const rb_data_type_t Http_data_type = {
    .wrap_struct_name = "SFML::Http",
    .function = {.dmark = NULL, .dfree = Http_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static const rb_data_type_t HttpRequest_data_type = {
    .wrap_struct_name = "SFML::HttpRequest",
    .function = {.dmark = NULL, .dfree = HttpRequest_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static const rb_data_type_t HttpResponse_data_type = {
    .wrap_struct_name = "SFML::HttpResponse",
    .function = {.dmark = NULL, .dfree = HttpResponse_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE Http_wrap(VALUE klass, sfHttp* handle) {
    Http* ptr;

    if (handle == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create HTTP client");
    }

    ptr = malloc(sizeof(Http));
    ptr->handle = handle;

    return TypedData_Wrap_Struct(klass, &Http_data_type, ptr);
}

static VALUE HttpRequest_wrap(VALUE klass, sfHttpRequest* handle) {
    HttpRequest* ptr;

    if (handle == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create HTTP request");
    }

    ptr = malloc(sizeof(HttpRequest));
    ptr->handle = handle;

    return TypedData_Wrap_Struct(klass, &HttpRequest_data_type, ptr);
}

static VALUE HttpResponse_wrap(sfHttpResponse* handle) {
    HttpResponse* ptr;

    if (handle == NULL) {
        rb_raise(rb_eRuntimeError, "HTTP request failed");
    }

    ptr = malloc(sizeof(HttpResponse));
    ptr->handle = handle;

    return TypedData_Wrap_Struct(rb_cHttpResponse, &HttpResponse_data_type, ptr);
}

/* call-seq:
 *   Http.new -> Http
 *
 * Creates a new HTTP client with no host set.
 *
 * @return [Http]
 */
static VALUE Http_new(VALUE klass) {
    return Http_wrap(klass, sfHttp_create());
}

/* call-seq:
 *   set_host(host, port = 0) -> self
 *
 * Sets the target host of subsequent requests. +port+ of 0 means the
 * standard port for the scheme (80 for HTTP, 443 for HTTPS) is used;
 * +host+ may include a scheme, e.g. +"https://example.com"+.
 *
 * @return [self]
 */
static VALUE Http_set_host(VALUE self, VALUE rb_host, VALUE rb_port) {
    sfHttp_setHost(Get_Http_Struct(self), StringValueCStr(rb_host),
                   (unsigned short)NUM2INT(rb_port));
    return self;
}

/* call-seq:
 *   send_request(request, timeout = Time.zero) -> HttpResponse
 *
 * Sends +request+ to the host set with #set_host, blocking until the
 * response arrives or +timeout+ elapses.
 *
 * @return [HttpResponse]
 * @raise [TypeError] if +request+ is not an SFML::HttpRequest
 */
static VALUE Http_send_request(int argc, VALUE* argv, VALUE self) {
    VALUE rb_request, rb_timeout;
    sfTime timeout = sfTime_Zero;

    rb_scan_args(argc, argv, "11", &rb_request, &rb_timeout);

    if (!rb_obj_is_kind_of(rb_request, rb_cHttpRequest)) {
        rb_raise(rb_eTypeError, "expected an SFML::HttpRequest");
    }

    if (!NIL_P(rb_timeout)) {
        timeout = time_from_rb(rb_timeout);
    }

    return HttpResponse_wrap(
        sfHttp_sendRequest(Get_Http_Struct(self), Get_HttpRequest_Struct(rb_request), timeout));
}

/* call-seq:
 *   HttpRequest.new -> HttpRequest
 *
 * Creates a new HTTP request.
 *
 * @return [HttpRequest] a new request, defaulting to a GET of +"/"+ over
 *   HTTP/1.0 with no fields or body
 */
static VALUE HttpRequest_new(VALUE klass) {
    return HttpRequest_wrap(klass, sfHttpRequest_create());
}

/* call-seq:
 *   set_field(field, value) -> value
 *
 * Sets the value of a header field, e.g. +"Content-Type"+. Field names are
 * case-insensitive. A few fields (Content-Length, Connection, User-Agent,
 * Host) are managed automatically and overwritten before sending.
 *
 * @return [String] +value+
 */
static VALUE HttpRequest_set_field(VALUE self, VALUE rb_field, VALUE rb_value) {
    sfHttpRequest_setField(Get_HttpRequest_Struct(self), StringValueCStr(rb_field),
                           StringValueCStr(rb_value));
    return rb_value;
}

/* call-seq:
 *   method=(value) -> value
 *
 * +value+ is an HttpMethod name or value.
 *
 * @return [Symbol, Integer] +value+
 */
static VALUE HttpRequest_set_method(VALUE self, VALUE rb_method) {
    sfHttpRequest_setMethod(Get_HttpRequest_Struct(self), http_method_from_rb(rb_method));
    return rb_method;
}

/* call-seq:
 *   uri=(value) -> value
 *
 * Sets the request URI, e.g. +"/index.html"+.
 *
 * @return [String] +value+
 */
static VALUE HttpRequest_set_uri(VALUE self, VALUE rb_uri) {
    sfHttpRequest_setUri(Get_HttpRequest_Struct(self), StringValueCStr(rb_uri));
    return rb_uri;
}

/* call-seq:
 *   set_http_version(major, minor) -> [major, minor]
 *
 * Sets the HTTP protocol version used by the request.
 *
 * @return [Array(Integer, Integer)] +[major, minor]+
 */
static VALUE HttpRequest_set_http_version(VALUE self, VALUE rb_major, VALUE rb_minor) {
    sfHttpRequest_setHttpVersion(Get_HttpRequest_Struct(self), (unsigned int)NUM2INT(rb_major),
                                 (unsigned int)NUM2INT(rb_minor));

    return rb_ary_new_from_args(2, rb_major, rb_minor);
}

/* call-seq:
 *   body=(value) -> value
 *
 * Sets the request body, used by POST/PUT requests; ignored for GET/HEAD.
 *
 * @return [String] +value+
 */
static VALUE HttpRequest_set_body(VALUE self, VALUE rb_body) {
    sfHttpRequest_setBody(Get_HttpRequest_Struct(self), StringValueCStr(rb_body));
    return rb_body;
}

/* call-seq:
 *   field(name) -> String or nil
 *
 * Returns the value of the response header field named +name+.
 *
 * @return [String, nil] the value of response header field +name+
 *   (case-insensitive), or +nil+ if it wasn't sent
 */
static VALUE HttpResponse_field(VALUE self, VALUE rb_field) {
    const char* field =
        sfHttpResponse_getField(Get_HttpResponse_Struct(self), StringValueCStr(rb_field));

    return field != NULL ? rb_str_new_cstr(field) : Qnil;
}

/* call-seq: status -> Integer
 *
 * Returns the numeric HTTP status code.
 *
 * @return [Integer] the raw HTTP status code
 */
static VALUE HttpResponse_status(VALUE self) {
    return INT2NUM(sfHttpResponse_getStatus(Get_HttpResponse_Struct(self)));
}

/* call-seq: status_name -> Symbol
 *
 * Returns the status as an HttpStatus name.
 *
 * @return [Symbol] an HttpStatus name for #status
 */
static VALUE HttpResponse_status_name(VALUE self) {
    return ID2SYM(
        rb_intern(http_status_name(sfHttpResponse_getStatus(Get_HttpResponse_Struct(self)))));
}

/* call-seq: major_version -> Integer
 *
 * Returns the major component of the HTTP protocol version.
 *
 * @return [Integer] the major version of the HTTP protocol used by the server
 */
static VALUE HttpResponse_major_version(VALUE self) {
    return UINT2NUM(sfHttpResponse_getMajorVersion(Get_HttpResponse_Struct(self)));
}

/* call-seq: minor_version -> Integer
 *
 * Returns the minor component of the HTTP protocol version.
 *
 * @return [Integer] the minor version of the HTTP protocol used by the server
 */
static VALUE HttpResponse_minor_version(VALUE self) {
    return UINT2NUM(sfHttpResponse_getMinorVersion(Get_HttpResponse_Struct(self)));
}

/* call-seq: body -> String
 *
 * Returns the body of the response.
 *
 * @return [String] the body of the response, possibly an error message
 *   when the server sent something other than valid HTML
 */
static VALUE HttpResponse_body(VALUE self) {
    return rb_str_new_cstr(sfHttpResponse_getBody(Get_HttpResponse_Struct(self)));
}

/* Document-class: SFML::Http
 * A basic HTTP client for performing requests against one host at a time
 * (set with #set_host).
 */
void Init_Http(VALUE rb_mSFML) {
    rb_cHttp = rb_define_class_under(rb_mSFML, "Http", rb_cObject);
    /* Document-class: SFML::HttpRequest
     * An HTTP request to send via Http#send_request.
     */
    rb_cHttpRequest = rb_define_class_under(rb_mSFML, "HttpRequest", rb_cObject);
    /* Document-class: SFML::HttpResponse
     * The status, headers and body returned by Http#send_request.
     */
    rb_cHttpResponse = rb_define_class_under(rb_mSFML, "HttpResponse", rb_cObject);

    rb_define_singleton_method(rb_cHttp, "new", Http_new, 0);
    rb_define_singleton_method(rb_cHttpRequest, "new", HttpRequest_new, 0);

    rb_define_method(rb_cHttp, "set_host", Http_set_host, 2);
    rb_define_method(rb_cHttp, "send_request", Http_send_request, -1);

    rb_define_method(rb_cHttpRequest, "set_field", HttpRequest_set_field, 2);
    rb_define_method(rb_cHttpRequest, "method=", HttpRequest_set_method, 1);
    rb_define_method(rb_cHttpRequest, "uri=", HttpRequest_set_uri, 1);
    rb_define_method(rb_cHttpRequest, "set_http_version", HttpRequest_set_http_version, 2);
    rb_define_method(rb_cHttpRequest, "body=", HttpRequest_set_body, 1);

    rb_define_method(rb_cHttpResponse, "field", HttpResponse_field, 1);
    rb_define_method(rb_cHttpResponse, "status", HttpResponse_status, 0);
    rb_define_method(rb_cHttpResponse, "status_name", HttpResponse_status_name, 0);
    rb_define_method(rb_cHttpResponse, "major_version", HttpResponse_major_version, 0);
    rb_define_method(rb_cHttpResponse, "minor_version", HttpResponse_minor_version, 0);
    rb_define_method(rb_cHttpResponse, "body", HttpResponse_body, 0);
}

VALUE Get_Klass_Http(void) {
    return rb_cHttp;
}

VALUE Get_Klass_HttpRequest(void) {
    return rb_cHttpRequest;
}

VALUE Get_Klass_HttpResponse(void) {
    return rb_cHttpResponse;
}

void* Get_Http_Struct(VALUE self) {
    Http* ptr;
    TypedData_Get_Struct(self, Http, &Http_data_type, ptr);
    return ptr->handle;
}

void* Get_HttpRequest_Struct(VALUE self) {
    HttpRequest* ptr;
    TypedData_Get_Struct(self, HttpRequest, &HttpRequest_data_type, ptr);
    return ptr->handle;
}

void* Get_HttpResponse_Struct(VALUE self) {
    HttpResponse* ptr;
    TypedData_Get_Struct(self, HttpResponse, &HttpResponse_data_type, ptr);
    return ptr->handle;
}
