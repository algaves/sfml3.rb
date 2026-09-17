#include "network/ftp.h"

#include <ruby.h>
#include <stdlib.h>

#include "core/macros.h"
#include "core/unicode.h"
#include "network/ip_address.h"
#include "network/network_enums.h"
#include "system/time.h"

typedef struct {
    sfFtp* handle;
} Ftp;

typedef struct {
    sfFtpResponse* handle;
} FtpResponse;

typedef struct {
    sfFtpDirectoryResponse* handle;
} FtpDirectoryResponse;

typedef struct {
    sfFtpListingResponse* handle;
} FtpListingResponse;

static VALUE rb_cFtp;
static VALUE rb_cFtpResponse;
static VALUE rb_cFtpDirectoryResponse;
static VALUE rb_cFtpListingResponse;

static void Ftp_free(void* ptr) {
    Ftp* ftp = ptr;

    sfFtp_destroy(ftp->handle);
    free(ftp);
}

static void FtpResponse_free(void* ptr) {
    FtpResponse* response = ptr;

    sfFtpResponse_destroy(response->handle);
    free(response);
}

static void FtpDirectoryResponse_free(void* ptr) {
    FtpDirectoryResponse* response = ptr;

    sfFtpDirectoryResponse_destroy(response->handle);
    free(response);
}

static void FtpListingResponse_free(void* ptr) {
    FtpListingResponse* response = ptr;

    sfFtpListingResponse_destroy(response->handle);
    free(response);
}

static const rb_data_type_t Ftp_data_type = {
    .wrap_struct_name = "SFML::Ftp",
    .function = {.dmark = NULL, .dfree = Ftp_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static const rb_data_type_t FtpResponse_data_type = {
    .wrap_struct_name = "SFML::FtpResponse",
    .function = {.dmark = NULL, .dfree = FtpResponse_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static const rb_data_type_t FtpDirectoryResponse_data_type = {
    .wrap_struct_name = "SFML::FtpDirectoryResponse",
    .function = {.dmark = NULL, .dfree = FtpDirectoryResponse_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static const rb_data_type_t FtpListingResponse_data_type = {
    .wrap_struct_name = "SFML::FtpListingResponse",
    .function = {.dmark = NULL, .dfree = FtpListingResponse_free, .dsize = NULL},
    .flags = RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE Ftp_wrap(sfFtp* handle) {
    Ftp* ptr;

    if (handle == NULL) {
        rb_raise(rb_eRuntimeError, "failed to create FTP client");
    }

    ptr = malloc(sizeof(Ftp));
    ptr->handle = handle;

    return TypedData_Wrap_Struct(rb_cFtp, &Ftp_data_type, ptr);
}

static VALUE FtpResponse_wrap(sfFtpResponse* handle) {
    FtpResponse* ptr;

    if (handle == NULL) {
        rb_raise(rb_eRuntimeError, "FTP request failed");
    }

    ptr = malloc(sizeof(FtpResponse));
    ptr->handle = handle;

    return TypedData_Wrap_Struct(rb_cFtpResponse, &FtpResponse_data_type, ptr);
}

static VALUE FtpDirectoryResponse_wrap(sfFtpDirectoryResponse* handle) {
    FtpDirectoryResponse* ptr;

    if (handle == NULL) {
        rb_raise(rb_eRuntimeError, "FTP request failed");
    }

    ptr = malloc(sizeof(FtpDirectoryResponse));
    ptr->handle = handle;

    return TypedData_Wrap_Struct(rb_cFtpDirectoryResponse, &FtpDirectoryResponse_data_type, ptr);
}

static VALUE FtpListingResponse_wrap(sfFtpListingResponse* handle) {
    FtpListingResponse* ptr;

    if (handle == NULL) {
        rb_raise(rb_eRuntimeError, "FTP request failed");
    }

    ptr = malloc(sizeof(FtpListingResponse));
    ptr->handle = handle;

    return TypedData_Wrap_Struct(rb_cFtpListingResponse, &FtpListingResponse_data_type, ptr);
}

static VALUE Ftp_new(VALUE klass) {
    return Ftp_wrap(sfFtp_create());
}

static VALUE Ftp_connect(int argc, VALUE* argv, VALUE self) {
    VALUE rb_address, rb_port, rb_timeout;
    sfTime timeout = sfTime_Zero;
    unsigned short port = 21;

    rb_scan_args(argc, argv, "12", &rb_address, &rb_port, &rb_timeout);

    if (!NIL_P(rb_port)) {
        port = (unsigned short)NUM2INT(rb_port);
    }

    if (!NIL_P(rb_timeout)) {
        timeout = time_from_rb(rb_timeout);
    }

    return FtpResponse_wrap(sfFtp_connect(
        Get_Ftp_Struct(self), ip_address_from_rb(rb_address, sfIpAddress_None), port, timeout));
}

static VALUE Ftp_login_anonymous(VALUE self) {
    return FtpResponse_wrap(sfFtp_loginAnonymous(Get_Ftp_Struct(self)));
}

static VALUE Ftp_login(VALUE self, VALUE rb_name, VALUE rb_password) {
    return FtpResponse_wrap(
        sfFtp_login(Get_Ftp_Struct(self), StringValueCStr(rb_name), StringValueCStr(rb_password)));
}

static VALUE Ftp_disconnect(VALUE self) {
    return FtpResponse_wrap(sfFtp_disconnect(Get_Ftp_Struct(self)));
}

static VALUE Ftp_keep_alive(VALUE self) {
    return FtpResponse_wrap(sfFtp_keepAlive(Get_Ftp_Struct(self)));
}

static VALUE Ftp_working_directory(VALUE self) {
    return FtpDirectoryResponse_wrap(sfFtp_getWorkingDirectory(Get_Ftp_Struct(self)));
}

static VALUE Ftp_directory_listing(int argc, VALUE* argv, VALUE self) {
    VALUE rb_directory;

    rb_scan_args(argc, argv, "01", &rb_directory);

    return FtpListingResponse_wrap(sfFtp_getDirectoryListing(
        Get_Ftp_Struct(self), NIL_P(rb_directory) ? NULL : StringValueCStr(rb_directory)));
}

static VALUE Ftp_change_directory(VALUE self, VALUE rb_directory) {
    return FtpResponse_wrap(
        sfFtp_changeDirectory(Get_Ftp_Struct(self), StringValueCStr(rb_directory)));
}

static VALUE Ftp_parent_directory(VALUE self) {
    return FtpResponse_wrap(sfFtp_parentDirectory(Get_Ftp_Struct(self)));
}

static VALUE Ftp_create_directory(VALUE self, VALUE rb_name) {
    return FtpResponse_wrap(sfFtp_createDirectory(Get_Ftp_Struct(self), StringValueCStr(rb_name)));
}

static VALUE Ftp_delete_directory(VALUE self, VALUE rb_name) {
    return FtpResponse_wrap(sfFtp_deleteDirectory(Get_Ftp_Struct(self), StringValueCStr(rb_name)));
}

static VALUE Ftp_rename_file(VALUE self, VALUE rb_file, VALUE rb_new_name) {
    return FtpResponse_wrap(sfFtp_renameFile(Get_Ftp_Struct(self), StringValueCStr(rb_file),
                                             StringValueCStr(rb_new_name)));
}

static VALUE Ftp_delete_file(VALUE self, VALUE rb_name) {
    return FtpResponse_wrap(sfFtp_deleteFile(Get_Ftp_Struct(self), StringValueCStr(rb_name)));
}

static VALUE Ftp_download(int argc, VALUE* argv, VALUE self) {
    VALUE rb_remote, rb_local, rb_mode;
    sfFtpTransferMode mode = sfFtpBinary;

    rb_scan_args(argc, argv, "21", &rb_remote, &rb_local, &rb_mode);

    if (!NIL_P(rb_mode)) {
        mode = ftp_transfer_mode_from_rb(rb_mode);
    }

    return FtpResponse_wrap(sfFtp_download(Get_Ftp_Struct(self), StringValueCStr(rb_remote),
                                           StringValueCStr(rb_local), mode));
}

static VALUE Ftp_upload(int argc, VALUE* argv, VALUE self) {
    VALUE rb_local, rb_remote, rb_mode, rb_append;
    sfFtpTransferMode mode = sfFtpBinary;
    bool append = false;

    rb_scan_args(argc, argv, "22", &rb_local, &rb_remote, &rb_mode, &rb_append);

    if (!NIL_P(rb_mode)) {
        mode = ftp_transfer_mode_from_rb(rb_mode);
    }

    if (!NIL_P(rb_append)) {
        append = RTEST(rb_append);
    }

    return FtpResponse_wrap(sfFtp_upload(Get_Ftp_Struct(self), StringValueCStr(rb_local),
                                         StringValueCStr(rb_remote), mode, append));
}

static VALUE Ftp_send_command(int argc, VALUE* argv, VALUE self) {
    VALUE rb_command, rb_parameter;

    rb_scan_args(argc, argv, "11", &rb_command, &rb_parameter);

    return FtpResponse_wrap(
        sfFtp_sendCommand(Get_Ftp_Struct(self), StringValueCStr(rb_command),
                          NIL_P(rb_parameter) ? NULL : StringValueCStr(rb_parameter)));
}

static VALUE FtpResponse_ok(VALUE self) {
    return BOOL2RB(sfFtpResponse_isOk(Get_FtpResponse_Struct(self)));
}

static VALUE FtpResponse_status(VALUE self) {
    return INT2NUM(sfFtpResponse_getStatus(Get_FtpResponse_Struct(self)));
}

static VALUE FtpResponse_status_name(VALUE self) {
    return ID2SYM(
        rb_intern(ftp_status_name(sfFtpResponse_getStatus(Get_FtpResponse_Struct(self)))));
}

static VALUE FtpResponse_message(VALUE self) {
    return rb_str_new_cstr(sfFtpResponse_getMessage(Get_FtpResponse_Struct(self)));
}

static VALUE FtpDirectoryResponse_ok(VALUE self) {
    return BOOL2RB(sfFtpDirectoryResponse_isOk(Get_FtpDirectoryResponse_Struct(self)));
}

static VALUE FtpDirectoryResponse_status(VALUE self) {
    return INT2NUM(sfFtpDirectoryResponse_getStatus(Get_FtpDirectoryResponse_Struct(self)));
}

static VALUE FtpDirectoryResponse_status_name(VALUE self) {
    return ID2SYM(rb_intern(
        ftp_status_name(sfFtpDirectoryResponse_getStatus(Get_FtpDirectoryResponse_Struct(self)))));
}

static VALUE FtpDirectoryResponse_message(VALUE self) {
    return rb_str_new_cstr(
        sfFtpDirectoryResponse_getMessage(Get_FtpDirectoryResponse_Struct(self)));
}

/* Through the UTF-32 entry point, so a path with non-ASCII characters survives
   the trip; the plain getter re-encodes it through the C locale. */
static VALUE FtpDirectoryResponse_directory(VALUE self) {
    return utf32_to_rb(
        sfFtpDirectoryResponse_getDirectoryUnicode(Get_FtpDirectoryResponse_Struct(self)));
}

static VALUE FtpListingResponse_ok(VALUE self) {
    return BOOL2RB(sfFtpListingResponse_isOk(Get_FtpListingResponse_Struct(self)));
}

static VALUE FtpListingResponse_status(VALUE self) {
    return INT2NUM(sfFtpListingResponse_getStatus(Get_FtpListingResponse_Struct(self)));
}

static VALUE FtpListingResponse_status_name(VALUE self) {
    return ID2SYM(rb_intern(
        ftp_status_name(sfFtpListingResponse_getStatus(Get_FtpListingResponse_Struct(self)))));
}

static VALUE FtpListingResponse_message(VALUE self) {
    return rb_str_new_cstr(sfFtpListingResponse_getMessage(Get_FtpListingResponse_Struct(self)));
}

static VALUE FtpListingResponse_count(VALUE self) {
    return SIZET2NUM(sfFtpListingResponse_getCount(Get_FtpListingResponse_Struct(self)));
}

static VALUE FtpListingResponse_name(VALUE self, VALUE rb_index) {
    return rb_str_new_cstr(sfFtpListingResponse_getName(Get_FtpListingResponse_Struct(self),
                                                        (size_t)NUM2SIZET(rb_index)));
}

void Init_Ftp(VALUE rb_module) {
    rb_cFtp = rb_define_class_under(rb_module, "Ftp", rb_cObject);
    rb_cFtpResponse = rb_define_class_under(rb_module, "FtpResponse", rb_cObject);
    rb_cFtpDirectoryResponse = rb_define_class_under(rb_module, "FtpDirectoryResponse", rb_cObject);
    rb_cFtpListingResponse = rb_define_class_under(rb_module, "FtpListingResponse", rb_cObject);

    rb_define_singleton_method(rb_cFtp, "new", Ftp_new, 0);

    rb_define_method(rb_cFtp, "connect", Ftp_connect, -1);
    rb_define_method(rb_cFtp, "login_anonymous", Ftp_login_anonymous, 0);
    rb_define_method(rb_cFtp, "login", Ftp_login, 2);
    rb_define_method(rb_cFtp, "disconnect", Ftp_disconnect, 0);
    rb_define_method(rb_cFtp, "keep_alive", Ftp_keep_alive, 0);
    rb_define_method(rb_cFtp, "working_directory", Ftp_working_directory, 0);
    rb_define_method(rb_cFtp, "directory_listing", Ftp_directory_listing, -1);
    rb_define_method(rb_cFtp, "change_directory", Ftp_change_directory, 1);
    rb_define_method(rb_cFtp, "parent_directory", Ftp_parent_directory, 0);
    rb_define_method(rb_cFtp, "create_directory", Ftp_create_directory, 1);
    rb_define_method(rb_cFtp, "delete_directory", Ftp_delete_directory, 1);
    rb_define_method(rb_cFtp, "rename_file", Ftp_rename_file, 2);
    rb_define_method(rb_cFtp, "delete_file", Ftp_delete_file, 1);
    rb_define_method(rb_cFtp, "download", Ftp_download, -1);
    rb_define_method(rb_cFtp, "upload", Ftp_upload, -1);
    rb_define_method(rb_cFtp, "send_command", Ftp_send_command, -1);

    rb_define_method(rb_cFtpResponse, "ok?", FtpResponse_ok, 0);
    rb_define_method(rb_cFtpResponse, "status", FtpResponse_status, 0);
    rb_define_method(rb_cFtpResponse, "status_name", FtpResponse_status_name, 0);
    rb_define_method(rb_cFtpResponse, "message", FtpResponse_message, 0);

    rb_define_method(rb_cFtpDirectoryResponse, "ok?", FtpDirectoryResponse_ok, 0);
    rb_define_method(rb_cFtpDirectoryResponse, "status", FtpDirectoryResponse_status, 0);
    rb_define_method(rb_cFtpDirectoryResponse, "status_name", FtpDirectoryResponse_status_name, 0);
    rb_define_method(rb_cFtpDirectoryResponse, "message", FtpDirectoryResponse_message, 0);
    rb_define_method(rb_cFtpDirectoryResponse, "directory", FtpDirectoryResponse_directory, 0);

    rb_define_method(rb_cFtpListingResponse, "ok?", FtpListingResponse_ok, 0);
    rb_define_method(rb_cFtpListingResponse, "status", FtpListingResponse_status, 0);
    rb_define_method(rb_cFtpListingResponse, "status_name", FtpListingResponse_status_name, 0);
    rb_define_method(rb_cFtpListingResponse, "message", FtpListingResponse_message, 0);
    rb_define_method(rb_cFtpListingResponse, "count", FtpListingResponse_count, 0);
    rb_define_method(rb_cFtpListingResponse, "name", FtpListingResponse_name, 1);
}

VALUE Get_Klass_Ftp(void) {
    return rb_cFtp;
}

VALUE Get_Klass_FtpResponse(void) {
    return rb_cFtpResponse;
}

VALUE Get_Klass_FtpDirectoryResponse(void) {
    return rb_cFtpDirectoryResponse;
}

VALUE Get_Klass_FtpListingResponse(void) {
    return rb_cFtpListingResponse;
}

void* Get_Ftp_Struct(VALUE self) {
    Ftp* ptr;
    TypedData_Get_Struct(self, Ftp, &Ftp_data_type, ptr);
    return ptr->handle;
}

void* Get_FtpResponse_Struct(VALUE self) {
    FtpResponse* ptr;
    TypedData_Get_Struct(self, FtpResponse, &FtpResponse_data_type, ptr);
    return ptr->handle;
}

void* Get_FtpDirectoryResponse_Struct(VALUE self) {
    FtpDirectoryResponse* ptr;
    TypedData_Get_Struct(self, FtpDirectoryResponse, &FtpDirectoryResponse_data_type, ptr);
    return ptr->handle;
}

void* Get_FtpListingResponse_Struct(VALUE self) {
    FtpListingResponse* ptr;
    TypedData_Get_Struct(self, FtpListingResponse, &FtpListingResponse_data_type, ptr);
    return ptr->handle;
}
