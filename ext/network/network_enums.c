#include "network/network_enums.h"

#include <ruby.h>
#include <string.h>

static const char *socket_status_names[] = {"done", "not_ready", "partial", "disconnected", "error"};
static const char *http_method_names[] = {"get", "post", "head", "put", "delete"};
static const char *ftp_transfer_mode_names[] = {"binary", "ascii", "ebcdic"};

typedef struct {
    int value;
    const char *name;
} EnumName;

/* The HTTP and FTP status enums are sparse and mostly meaningful by number,
   so they are stored as value/name pairs and looked up linearly. */
static const EnumName http_status_names[] = {
    {sfHttpOk, "ok"},
    {sfHttpCreated, "created"},
    {sfHttpAccepted, "accepted"},
    {sfHttpNoContent, "no_content"},
    {sfHttpResetContent, "reset_content"},
    {sfHttpPartialContent, "partial_content"},
    {sfHttpMultipleChoices, "multiple_choices"},
    {sfHttpMovedPermanently, "moved_permanently"},
    {sfHttpMovedTemporarily, "moved_temporarily"},
    {sfHttpNotModified, "not_modified"},
    {sfHttpBadRequest, "bad_request"},
    {sfHttpUnauthorized, "unauthorized"},
    {sfHttpForbidden, "forbidden"},
    {sfHttpNotFound, "not_found"},
    {sfHttpRangeNotSatisfiable, "range_not_satisfiable"},
    {sfHttpInternalServerError, "internal_server_error"},
    {sfHttpNotImplemented, "not_implemented"},
    {sfHttpBadGateway, "bad_gateway"},
    {sfHttpServiceNotAvailable, "service_not_available"},
    {sfHttpGatewayTimeout, "gateway_timeout"},
    {sfHttpVersionNotSupported, "version_not_supported"},
    {sfHttpInvalidResponse, "invalid_response"},
    {sfHttpConnectionFailed, "connection_failed"}
};

static const EnumName ftp_status_names[] = {
    {sfFtpRestartMarkerReply, "restart_marker_reply"},
    {sfFtpServiceReadySoon, "service_ready_soon"},
    {sfFtpDataConnectionAlreadyOpened, "data_connection_already_opened"},
    {sfFtpOpeningDataConnection, "opening_data_connection"},
    {sfFtpOk, "ok"},
    {sfFtpPointlessCommand, "pointless_command"},
    {sfFtpSystemStatus, "system_status"},
    {sfFtpDirectoryStatus, "directory_status"},
    {sfFtpFileStatus, "file_status"},
    {sfFtpHelpMessage, "help_message"},
    {sfFtpSystemType, "system_type"},
    {sfFtpServiceReady, "service_ready"},
    {sfFtpClosingConnection, "closing_connection"},
    {sfFtpDataConnectionOpened, "data_connection_opened"},
    {sfFtpClosingDataConnection, "closing_data_connection"},
    {sfFtpEnteringPassiveMode, "entering_passive_mode"},
    {sfFtpLoggedIn, "logged_in"},
    {sfFtpFileActionOk, "file_action_ok"},
    {sfFtpDirectoryOk, "directory_ok"},
    {sfFtpNeedPassword, "need_password"},
    {sfFtpNeedAccountToLogIn, "need_account_to_log_in"},
    {sfFtpNeedInformation, "need_information"},
    {sfFtpServiceUnavailable, "service_unavailable"},
    {sfFtpDataConnectionUnavailable, "data_connection_unavailable"},
    {sfFtpTransferAborted, "transfer_aborted"},
    {sfFtpFileActionAborted, "file_action_aborted"},
    {sfFtpLocalError, "local_error"},
    {sfFtpInsufficientStorageSpace, "insufficient_storage_space"},
    {sfFtpCommandUnknown, "command_unknown"},
    {sfFtpParametersUnknown, "parameters_unknown"},
    {sfFtpCommandNotImplemented, "command_not_implemented"},
    {sfFtpBadCommandSequence, "bad_command_sequence"},
    {sfFtpParameterNotImplemented, "parameter_not_implemented"},
    {sfFtpNotLoggedIn, "not_logged_in"},
    {sfFtpNeedAccountToStore, "need_account_to_store"},
    {sfFtpFileUnavailable, "file_unavailable"},
    {sfFtpPageTypeUnknown, "page_type_unknown"},
    {sfFtpNotEnoughMemory, "not_enough_memory"},
    {sfFtpFilenameNotAllowed, "filename_not_allowed"},
    {sfFtpInvalidResponse, "invalid_response"},
    {sfFtpConnectionFailed, "connection_failed"},
    {sfFtpConnectionClosed, "connection_closed"},
    {sfFtpInvalidFile, "invalid_file"}
};

static int symbol_index(VALUE rb_symbol, const char *const *names, size_t count) {
    const char *name;
    size_t i;

    if (!SYMBOL_P(rb_symbol)) {
        return -1;
    }

    name = rb_id2name(SYM2ID(rb_symbol));

    for (i = 0; i < count; i++) {
        if (strcmp(name, names[i]) == 0) {
            return (int) i;
        }
    }

    return -1;
}

const char *socket_status_name(sfSocketStatus status) {
    if (status >= sfSocketDone && status <= sfSocketError) {
        return socket_status_names[status];
    }

    return "error";
}

sfSocketStatus socket_status_from_rb(VALUE rb_status) {
    int index;

    if (RB_INTEGER_TYPE_P(rb_status)) {
        return (sfSocketStatus) NUM2INT(rb_status);
    }

    index = symbol_index(rb_status, socket_status_names, 5);

    if (index < 0) {
        rb_raise(rb_eArgError, "unknown socket status");
    }

    return (sfSocketStatus) index;
}

const char *http_method_name(sfHttpMethod method) {
    if (method >= sfHttpGet && method <= sfHttpDelete) {
        return http_method_names[method];
    }

    return "get";
}

sfHttpMethod http_method_from_rb(VALUE rb_method) {
    int index;

    if (RB_INTEGER_TYPE_P(rb_method)) {
        return (sfHttpMethod) NUM2INT(rb_method);
    }

    index = symbol_index(rb_method, http_method_names, 5);

    if (index < 0) {
        rb_raise(rb_eArgError, "unknown HTTP method");
    }

    return (sfHttpMethod) index;
}

static const char *lookup_name(const EnumName *table, size_t count, int value) {
    size_t i;

    for (i = 0; i < count; i++) {
        if (table[i].value == value) {
            return table[i].name;
        }
    }

    return NULL;
}

const char *http_status_name(sfHttpStatus status) {
    const char *name = lookup_name(http_status_names,
                                   sizeof(http_status_names) / sizeof(http_status_names[0]), status);

    return name != NULL ? name : "unknown";
}

const char *ftp_transfer_mode_name(sfFtpTransferMode mode) {
    if (mode >= sfFtpBinary && mode <= sfFtpEbcdic) {
        return ftp_transfer_mode_names[mode];
    }

    return "binary";
}

sfFtpTransferMode ftp_transfer_mode_from_rb(VALUE rb_mode) {
    int index;

    if (RB_INTEGER_TYPE_P(rb_mode)) {
        return (sfFtpTransferMode) NUM2INT(rb_mode);
    }

    index = symbol_index(rb_mode, ftp_transfer_mode_names, 3);

    if (index < 0) {
        rb_raise(rb_eArgError, "unknown FTP transfer mode");
    }

    return (sfFtpTransferMode) index;
}

const char *ftp_status_name(sfFtpStatus status) {
    const char *name = lookup_name(ftp_status_names,
                                   sizeof(ftp_status_names) / sizeof(ftp_status_names[0]), status);

    return name != NULL ? name : "unknown";
}

#define DEFINE_CONST(module, name, value) rb_define_const(module, name, INT2NUM(value))

void Init_NetworkEnums(VALUE rb_module) {
    VALUE rb_mSocketStatus = rb_define_module_under(rb_module, "SocketStatus");
    VALUE rb_mHttpMethod = rb_define_module_under(rb_module, "HttpMethod");
    VALUE rb_mHttpStatus = rb_define_module_under(rb_module, "HttpStatus");
    VALUE rb_mFtpTransferMode = rb_define_module_under(rb_module, "FtpTransferMode");
    VALUE rb_mFtpStatus = rb_define_module_under(rb_module, "FtpStatus");

    DEFINE_CONST(rb_mSocketStatus, "DONE", sfSocketDone);
    DEFINE_CONST(rb_mSocketStatus, "NOT_READY", sfSocketNotReady);
    DEFINE_CONST(rb_mSocketStatus, "PARTIAL", sfSocketPartial);
    DEFINE_CONST(rb_mSocketStatus, "DISCONNECTED", sfSocketDisconnected);
    DEFINE_CONST(rb_mSocketStatus, "ERROR", sfSocketError);

    DEFINE_CONST(rb_mHttpMethod, "GET", sfHttpGet);
    DEFINE_CONST(rb_mHttpMethod, "POST", sfHttpPost);
    DEFINE_CONST(rb_mHttpMethod, "HEAD", sfHttpHead);
    DEFINE_CONST(rb_mHttpMethod, "PUT", sfHttpPut);
    DEFINE_CONST(rb_mHttpMethod, "DELETE", sfHttpDelete);

    DEFINE_CONST(rb_mHttpStatus, "OK", sfHttpOk);
    DEFINE_CONST(rb_mHttpStatus, "CREATED", sfHttpCreated);
    DEFINE_CONST(rb_mHttpStatus, "ACCEPTED", sfHttpAccepted);
    DEFINE_CONST(rb_mHttpStatus, "NO_CONTENT", sfHttpNoContent);
    DEFINE_CONST(rb_mHttpStatus, "RESET_CONTENT", sfHttpResetContent);
    DEFINE_CONST(rb_mHttpStatus, "PARTIAL_CONTENT", sfHttpPartialContent);
    DEFINE_CONST(rb_mHttpStatus, "MULTIPLE_CHOICES", sfHttpMultipleChoices);
    DEFINE_CONST(rb_mHttpStatus, "MOVED_PERMANENTLY", sfHttpMovedPermanently);
    DEFINE_CONST(rb_mHttpStatus, "MOVED_TEMPORARILY", sfHttpMovedTemporarily);
    DEFINE_CONST(rb_mHttpStatus, "NOT_MODIFIED", sfHttpNotModified);
    DEFINE_CONST(rb_mHttpStatus, "BAD_REQUEST", sfHttpBadRequest);
    DEFINE_CONST(rb_mHttpStatus, "UNAUTHORIZED", sfHttpUnauthorized);
    DEFINE_CONST(rb_mHttpStatus, "FORBIDDEN", sfHttpForbidden);
    DEFINE_CONST(rb_mHttpStatus, "NOT_FOUND", sfHttpNotFound);
    DEFINE_CONST(rb_mHttpStatus, "RANGE_NOT_SATISFIABLE", sfHttpRangeNotSatisfiable);
    DEFINE_CONST(rb_mHttpStatus, "INTERNAL_SERVER_ERROR", sfHttpInternalServerError);
    DEFINE_CONST(rb_mHttpStatus, "NOT_IMPLEMENTED", sfHttpNotImplemented);
    DEFINE_CONST(rb_mHttpStatus, "BAD_GATEWAY", sfHttpBadGateway);
    DEFINE_CONST(rb_mHttpStatus, "SERVICE_NOT_AVAILABLE", sfHttpServiceNotAvailable);
    DEFINE_CONST(rb_mHttpStatus, "GATEWAY_TIMEOUT", sfHttpGatewayTimeout);
    DEFINE_CONST(rb_mHttpStatus, "VERSION_NOT_SUPPORTED", sfHttpVersionNotSupported);
    DEFINE_CONST(rb_mHttpStatus, "INVALID_RESPONSE", sfHttpInvalidResponse);
    DEFINE_CONST(rb_mHttpStatus, "CONNECTION_FAILED", sfHttpConnectionFailed);

    DEFINE_CONST(rb_mFtpTransferMode, "BINARY", sfFtpBinary);
    DEFINE_CONST(rb_mFtpTransferMode, "ASCII", sfFtpAscii);
    DEFINE_CONST(rb_mFtpTransferMode, "EBCDIC", sfFtpEbcdic);

    DEFINE_CONST(rb_mFtpStatus, "RESTART_MARKER_REPLY", sfFtpRestartMarkerReply);
    DEFINE_CONST(rb_mFtpStatus, "SERVICE_READY_SOON", sfFtpServiceReadySoon);
    DEFINE_CONST(rb_mFtpStatus, "DATA_CONNECTION_ALREADY_OPENED", sfFtpDataConnectionAlreadyOpened);
    DEFINE_CONST(rb_mFtpStatus, "OPENING_DATA_CONNECTION", sfFtpOpeningDataConnection);
    DEFINE_CONST(rb_mFtpStatus, "OK", sfFtpOk);
    DEFINE_CONST(rb_mFtpStatus, "POINTLESS_COMMAND", sfFtpPointlessCommand);
    DEFINE_CONST(rb_mFtpStatus, "SYSTEM_STATUS", sfFtpSystemStatus);
    DEFINE_CONST(rb_mFtpStatus, "DIRECTORY_STATUS", sfFtpDirectoryStatus);
    DEFINE_CONST(rb_mFtpStatus, "FILE_STATUS", sfFtpFileStatus);
    DEFINE_CONST(rb_mFtpStatus, "HELP_MESSAGE", sfFtpHelpMessage);
    DEFINE_CONST(rb_mFtpStatus, "SYSTEM_TYPE", sfFtpSystemType);
    DEFINE_CONST(rb_mFtpStatus, "SERVICE_READY", sfFtpServiceReady);
    DEFINE_CONST(rb_mFtpStatus, "CLOSING_CONNECTION", sfFtpClosingConnection);
    DEFINE_CONST(rb_mFtpStatus, "DATA_CONNECTION_OPENED", sfFtpDataConnectionOpened);
    DEFINE_CONST(rb_mFtpStatus, "CLOSING_DATA_CONNECTION", sfFtpClosingDataConnection);
    DEFINE_CONST(rb_mFtpStatus, "ENTERING_PASSIVE_MODE", sfFtpEnteringPassiveMode);
    DEFINE_CONST(rb_mFtpStatus, "LOGGED_IN", sfFtpLoggedIn);
    DEFINE_CONST(rb_mFtpStatus, "FILE_ACTION_OK", sfFtpFileActionOk);
    DEFINE_CONST(rb_mFtpStatus, "DIRECTORY_OK", sfFtpDirectoryOk);
    DEFINE_CONST(rb_mFtpStatus, "NEED_PASSWORD", sfFtpNeedPassword);
    DEFINE_CONST(rb_mFtpStatus, "NEED_ACCOUNT_TO_LOG_IN", sfFtpNeedAccountToLogIn);
    DEFINE_CONST(rb_mFtpStatus, "NEED_INFORMATION", sfFtpNeedInformation);
    DEFINE_CONST(rb_mFtpStatus, "SERVICE_UNAVAILABLE", sfFtpServiceUnavailable);
    DEFINE_CONST(rb_mFtpStatus, "DATA_CONNECTION_UNAVAILABLE", sfFtpDataConnectionUnavailable);
    DEFINE_CONST(rb_mFtpStatus, "TRANSFER_ABORTED", sfFtpTransferAborted);
    DEFINE_CONST(rb_mFtpStatus, "FILE_ACTION_ABORTED", sfFtpFileActionAborted);
    DEFINE_CONST(rb_mFtpStatus, "LOCAL_ERROR", sfFtpLocalError);
    DEFINE_CONST(rb_mFtpStatus, "INSUFFICIENT_STORAGE_SPACE", sfFtpInsufficientStorageSpace);
    DEFINE_CONST(rb_mFtpStatus, "COMMAND_UNKNOWN", sfFtpCommandUnknown);
    DEFINE_CONST(rb_mFtpStatus, "PARAMETERS_UNKNOWN", sfFtpParametersUnknown);
    DEFINE_CONST(rb_mFtpStatus, "COMMAND_NOT_IMPLEMENTED", sfFtpCommandNotImplemented);
    DEFINE_CONST(rb_mFtpStatus, "BAD_COMMAND_SEQUENCE", sfFtpBadCommandSequence);
    DEFINE_CONST(rb_mFtpStatus, "PARAMETER_NOT_IMPLEMENTED", sfFtpParameterNotImplemented);
    DEFINE_CONST(rb_mFtpStatus, "NOT_LOGGED_IN", sfFtpNotLoggedIn);
    DEFINE_CONST(rb_mFtpStatus, "NEED_ACCOUNT_TO_STORE", sfFtpNeedAccountToStore);
    DEFINE_CONST(rb_mFtpStatus, "FILE_UNAVAILABLE", sfFtpFileUnavailable);
    DEFINE_CONST(rb_mFtpStatus, "PAGE_TYPE_UNKNOWN", sfFtpPageTypeUnknown);
    DEFINE_CONST(rb_mFtpStatus, "NOT_ENOUGH_MEMORY", sfFtpNotEnoughMemory);
    DEFINE_CONST(rb_mFtpStatus, "FILENAME_NOT_ALLOWED", sfFtpFilenameNotAllowed);
    DEFINE_CONST(rb_mFtpStatus, "INVALID_RESPONSE", sfFtpInvalidResponse);
    DEFINE_CONST(rb_mFtpStatus, "CONNECTION_FAILED", sfFtpConnectionFailed);
    DEFINE_CONST(rb_mFtpStatus, "CONNECTION_CLOSED", sfFtpConnectionClosed);
    DEFINE_CONST(rb_mFtpStatus, "INVALID_FILE", sfFtpInvalidFile);
}
