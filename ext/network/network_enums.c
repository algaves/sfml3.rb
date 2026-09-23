#include "network/network_enums.h"

#include <ruby.h>
#include <string.h>

static const char* socket_status_names[] = {"done", "not_ready", "partial", "disconnected",
                                            "error"};
static const char* http_method_names[] = {"get", "post", "head", "put", "delete"};
static const char* ftp_transfer_mode_names[] = {"binary", "ascii", "ebcdic"};

typedef struct {
    int value;
    const char* name;
} EnumName;

/* The HTTP and FTP status enums are sparse and mostly meaningful by number,
   so they are stored as value/name pairs and looked up linearly. */
static const EnumName http_status_names[] = {{sfHttpOk, "ok"},
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
                                             {sfHttpConnectionFailed, "connection_failed"}};

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
    {sfFtpInvalidFile, "invalid_file"}};

static int symbol_index(VALUE rb_symbol, const char* const* names, size_t count) {
    const char* name;
    size_t i;

    if (!SYMBOL_P(rb_symbol)) {
        return -1;
    }

    name = rb_id2name(SYM2ID(rb_symbol));

    for (i = 0; i < count; i++) {
        if (strcmp(name, names[i]) == 0) {
            return (int)i;
        }
    }

    return -1;
}

const char* socket_status_name(sfSocketStatus status) {
    if (status >= sfSocketDone && status <= sfSocketError) {
        return socket_status_names[status];
    }

    return "error";
}

sfSocketStatus socket_status_from_rb(VALUE rb_status) {
    int index;

    if (RB_INTEGER_TYPE_P(rb_status)) {
        return (sfSocketStatus)NUM2INT(rb_status);
    }

    index = symbol_index(rb_status, socket_status_names, 5);

    if (index < 0) {
        rb_raise(rb_eArgError, "unknown socket status");
    }

    return (sfSocketStatus)index;
}

const char* http_method_name(sfHttpMethod method) {
    if (method >= sfHttpGet && method <= sfHttpDelete) {
        return http_method_names[method];
    }

    return "get";
}

sfHttpMethod http_method_from_rb(VALUE rb_method) {
    int index;

    if (RB_INTEGER_TYPE_P(rb_method)) {
        return (sfHttpMethod)NUM2INT(rb_method);
    }

    index = symbol_index(rb_method, http_method_names, 5);

    if (index < 0) {
        rb_raise(rb_eArgError, "unknown HTTP method");
    }

    return (sfHttpMethod)index;
}

static const char* lookup_name(const EnumName* table, size_t count, int value) {
    size_t i;

    for (i = 0; i < count; i++) {
        if (table[i].value == value) {
            return table[i].name;
        }
    }

    return NULL;
}

const char* http_status_name(sfHttpStatus status) {
    const char* name = lookup_name(
        http_status_names, sizeof(http_status_names) / sizeof(http_status_names[0]), status);

    return name != NULL ? name : "unknown";
}

const char* ftp_transfer_mode_name(sfFtpTransferMode mode) {
    if (mode >= sfFtpBinary && mode <= sfFtpEbcdic) {
        return ftp_transfer_mode_names[mode];
    }

    return "binary";
}

sfFtpTransferMode ftp_transfer_mode_from_rb(VALUE rb_mode) {
    int index;

    if (RB_INTEGER_TYPE_P(rb_mode)) {
        return (sfFtpTransferMode)NUM2INT(rb_mode);
    }

    index = symbol_index(rb_mode, ftp_transfer_mode_names, 3);

    if (index < 0) {
        rb_raise(rb_eArgError, "unknown FTP transfer mode");
    }

    return (sfFtpTransferMode)index;
}

const char* ftp_status_name(sfFtpStatus status) {
    const char* name = lookup_name(ftp_status_names,
                                   sizeof(ftp_status_names) / sizeof(ftp_status_names[0]), status);

    return name != NULL ? name : "unknown";
}

void Init_NetworkEnums(VALUE rb_mNetwork) {
    /* Document-module: SF::Network::SocketStatus
     * Status codes returned by socket send/receive/connect operations.
     */
    VALUE rb_mSocketStatus = rb_define_module_under(rb_mNetwork, "SocketStatus");

    /* Document-module: SF::Network::HttpMethod
     * The method (verb) of an HttpRequest.
     */
    VALUE rb_mHttpMethod = rb_define_module_under(rb_mNetwork, "HttpMethod");

    /* Document-module: SF::Network::HttpStatus
     * Status codes returned by an HttpResponse, mirroring standard HTTP
     * status codes with a few SFML-specific additions (1000+) for local
     * connection failures.
     */
    VALUE rb_mHttpStatus = rb_define_module_under(rb_mNetwork, "HttpStatus");

    /* Document-module: SF::Network::FtpTransferMode
     * The data transfer mode used by Ftp#download and Ftp#upload.
     */
    VALUE rb_mFtpTransferMode = rb_define_module_under(rb_mNetwork, "FtpTransferMode");

    /* Document-module: SF::Network::FtpStatus
     * Status codes returned by an FtpResponse, mirroring standard FTP
     * reply codes with a few SFML-specific additions (1000+) for local
     * connection failures.
     */
    VALUE rb_mFtpStatus = rb_define_module_under(rb_mNetwork, "FtpStatus");

    /* The socket has sent or received the data. */
    rb_define_const(rb_mSocketStatus, "DONE", INT2NUM(sfSocketDone));
    /* The socket is not ready to send or receive data yet. */
    rb_define_const(rb_mSocketStatus, "NOT_READY", INT2NUM(sfSocketNotReady));
    /* The socket sent a part of the data. */
    rb_define_const(rb_mSocketStatus, "PARTIAL", INT2NUM(sfSocketPartial));
    /* The TCP socket has been disconnected. */
    rb_define_const(rb_mSocketStatus, "DISCONNECTED", INT2NUM(sfSocketDisconnected));
    /* An unexpected error happened. */
    rb_define_const(rb_mSocketStatus, "ERROR", INT2NUM(sfSocketError));

    /* Request in GET mode, the standard method to retrieve a page. */
    rb_define_const(rb_mHttpMethod, "GET", INT2NUM(sfHttpGet));
    /* Request in POST mode, usually to send data to a page. */
    rb_define_const(rb_mHttpMethod, "POST", INT2NUM(sfHttpPost));
    /* Request a page's header only. */
    rb_define_const(rb_mHttpMethod, "HEAD", INT2NUM(sfHttpHead));
    /* Request in PUT mode, useful for a REST API. */
    rb_define_const(rb_mHttpMethod, "PUT", INT2NUM(sfHttpPut));
    /* Request in DELETE mode, useful for a REST API. */
    rb_define_const(rb_mHttpMethod, "DELETE", INT2NUM(sfHttpDelete));

    /* 200: most common code, returned when the operation was successful. */
    rb_define_const(rb_mHttpStatus, "OK", INT2NUM(sfHttpOk));
    /* 201: the resource has successfully been created. */
    rb_define_const(rb_mHttpStatus, "CREATED", INT2NUM(sfHttpCreated));
    /* 202: the request has been accepted, but will be processed later by the server. */
    rb_define_const(rb_mHttpStatus, "ACCEPTED", INT2NUM(sfHttpAccepted));
    /* 204: the server didn't send any data in return. */
    rb_define_const(rb_mHttpStatus, "NO_CONTENT", INT2NUM(sfHttpNoContent));
    /* 205: the server informs the client that it should clear the view (form) that caused the
     * request to be sent. */
    rb_define_const(rb_mHttpStatus, "RESET_CONTENT", INT2NUM(sfHttpResetContent));
    /* 206: the server has sent a part of the resource, as a response to a partial GET request. */
    rb_define_const(rb_mHttpStatus, "PARTIAL_CONTENT", INT2NUM(sfHttpPartialContent));
    /* 300: the requested page can be accessed from several locations. */
    rb_define_const(rb_mHttpStatus, "MULTIPLE_CHOICES", INT2NUM(sfHttpMultipleChoices));
    /* 301: the requested page has permanently moved to a new location. */
    rb_define_const(rb_mHttpStatus, "MOVED_PERMANENTLY", INT2NUM(sfHttpMovedPermanently));
    /* 302: the requested page has temporarily moved to a new location. */
    rb_define_const(rb_mHttpStatus, "MOVED_TEMPORARILY", INT2NUM(sfHttpMovedTemporarily));
    /* 304: for conditional requests, the requested page hasn't changed and doesn't need to be
     * refreshed. */
    rb_define_const(rb_mHttpStatus, "NOT_MODIFIED", INT2NUM(sfHttpNotModified));
    /* 400: the server couldn't understand the request (syntax error). */
    rb_define_const(rb_mHttpStatus, "BAD_REQUEST", INT2NUM(sfHttpBadRequest));
    /* 401: the requested page needs an authentication to be accessed. */
    rb_define_const(rb_mHttpStatus, "UNAUTHORIZED", INT2NUM(sfHttpUnauthorized));
    /* 403: the requested page cannot be accessed at all, even with authentication. */
    rb_define_const(rb_mHttpStatus, "FORBIDDEN", INT2NUM(sfHttpForbidden));
    /* 404: the requested page doesn't exist. */
    rb_define_const(rb_mHttpStatus, "NOT_FOUND", INT2NUM(sfHttpNotFound));
    /* 407: the server can't satisfy the partial GET request (with a "Range" header field). */
    rb_define_const(rb_mHttpStatus, "RANGE_NOT_SATISFIABLE", INT2NUM(sfHttpRangeNotSatisfiable));
    /* 500: the server encountered an unexpected error. */
    rb_define_const(rb_mHttpStatus, "INTERNAL_SERVER_ERROR", INT2NUM(sfHttpInternalServerError));
    /* 501: the server doesn't implement this request method. */
    rb_define_const(rb_mHttpStatus, "NOT_IMPLEMENTED", INT2NUM(sfHttpNotImplemented));
    /* 502: the gateway server has received an error from the source server. */
    rb_define_const(rb_mHttpStatus, "BAD_GATEWAY", INT2NUM(sfHttpBadGateway));
    /* 503: the service has not been found. */
    rb_define_const(rb_mHttpStatus, "SERVICE_NOT_AVAILABLE", INT2NUM(sfHttpServiceNotAvailable));
    /* 504: the gateway server didn't receive a response from the source server in a proper delay.
     */
    rb_define_const(rb_mHttpStatus, "GATEWAY_TIMEOUT", INT2NUM(sfHttpGatewayTimeout));
    /* 505: the server doesn't support the version of the HTTP protocol used. */
    rb_define_const(rb_mHttpStatus, "VERSION_NOT_SUPPORTED", INT2NUM(sfHttpVersionNotSupported));
    /* 1000 (SFML-specific): the response is not a valid HTTP one. */
    rb_define_const(rb_mHttpStatus, "INVALID_RESPONSE", INT2NUM(sfHttpInvalidResponse));
    /* 1001 (SFML-specific): the connection with the server failed. */
    rb_define_const(rb_mHttpStatus, "CONNECTION_FAILED", INT2NUM(sfHttpConnectionFailed));

    /* Binary transfer mode: files are transferred as raw bytes, unmodified. */
    rb_define_const(rb_mFtpTransferMode, "BINARY", INT2NUM(sfFtpBinary));
    /* Text (ASCII) transfer mode: files are converted to/from the text format of the receiving
     * system. */
    rb_define_const(rb_mFtpTransferMode, "ASCII", INT2NUM(sfFtpAscii));
    /* EBCDIC text transfer mode. */
    rb_define_const(rb_mFtpTransferMode, "EBCDIC", INT2NUM(sfFtpEbcdic));

    /* 110: restart marker reply. */
    rb_define_const(rb_mFtpStatus, "RESTART_MARKER_REPLY", INT2NUM(sfFtpRestartMarkerReply));
    /* 120: service ready in N minutes. */
    rb_define_const(rb_mFtpStatus, "SERVICE_READY_SOON", INT2NUM(sfFtpServiceReadySoon));
    /* 125: data connection already opened, transfer starting. */
    rb_define_const(rb_mFtpStatus, "DATA_CONNECTION_ALREADY_OPENED",
                    INT2NUM(sfFtpDataConnectionAlreadyOpened));
    /* 150: file status ok, about to open data connection. */
    rb_define_const(rb_mFtpStatus, "OPENING_DATA_CONNECTION", INT2NUM(sfFtpOpeningDataConnection));
    /* 200: command ok. */
    rb_define_const(rb_mFtpStatus, "OK", INT2NUM(sfFtpOk));
    /* 202: command not implemented. */
    rb_define_const(rb_mFtpStatus, "POINTLESS_COMMAND", INT2NUM(sfFtpPointlessCommand));
    /* 211: system status, or system help reply. */
    rb_define_const(rb_mFtpStatus, "SYSTEM_STATUS", INT2NUM(sfFtpSystemStatus));
    /* 212: directory status. */
    rb_define_const(rb_mFtpStatus, "DIRECTORY_STATUS", INT2NUM(sfFtpDirectoryStatus));
    /* 213: file status. */
    rb_define_const(rb_mFtpStatus, "FILE_STATUS", INT2NUM(sfFtpFileStatus));
    /* 214: help message. */
    rb_define_const(rb_mFtpStatus, "HELP_MESSAGE", INT2NUM(sfFtpHelpMessage));
    /* 215: NAME system type, where NAME is an official system name from the Assigned Numbers
     * document. */
    rb_define_const(rb_mFtpStatus, "SYSTEM_TYPE", INT2NUM(sfFtpSystemType));
    /* 220: service ready for a new user. */
    rb_define_const(rb_mFtpStatus, "SERVICE_READY", INT2NUM(sfFtpServiceReady));
    /* 221: service closing the control connection. */
    rb_define_const(rb_mFtpStatus, "CLOSING_CONNECTION", INT2NUM(sfFtpClosingConnection));
    /* 225: data connection open, no transfer in progress. */
    rb_define_const(rb_mFtpStatus, "DATA_CONNECTION_OPENED", INT2NUM(sfFtpDataConnectionOpened));
    /* 226: closing data connection, requested file action successful. */
    rb_define_const(rb_mFtpStatus, "CLOSING_DATA_CONNECTION", INT2NUM(sfFtpClosingDataConnection));
    /* 227: entering passive mode. */
    rb_define_const(rb_mFtpStatus, "ENTERING_PASSIVE_MODE", INT2NUM(sfFtpEnteringPassiveMode));
    /* 230: user logged in, proceed (logged out if appropriate). */
    rb_define_const(rb_mFtpStatus, "LOGGED_IN", INT2NUM(sfFtpLoggedIn));
    /* 250: requested file action ok. */
    rb_define_const(rb_mFtpStatus, "FILE_ACTION_OK", INT2NUM(sfFtpFileActionOk));
    /* 257: PATHNAME created. */
    rb_define_const(rb_mFtpStatus, "DIRECTORY_OK", INT2NUM(sfFtpDirectoryOk));
    /* 331: user name ok, password needed. */
    rb_define_const(rb_mFtpStatus, "NEED_PASSWORD", INT2NUM(sfFtpNeedPassword));
    /* 332: account needed for login. */
    rb_define_const(rb_mFtpStatus, "NEED_ACCOUNT_TO_LOG_IN", INT2NUM(sfFtpNeedAccountToLogIn));
    /* 350: requested file action pending further information. */
    rb_define_const(rb_mFtpStatus, "NEED_INFORMATION", INT2NUM(sfFtpNeedInformation));
    /* 421: service not available, closing the control connection. */
    rb_define_const(rb_mFtpStatus, "SERVICE_UNAVAILABLE", INT2NUM(sfFtpServiceUnavailable));
    /* 425: can't open the data connection. */
    rb_define_const(rb_mFtpStatus, "DATA_CONNECTION_UNAVAILABLE",
                    INT2NUM(sfFtpDataConnectionUnavailable));
    /* 426: connection closed, transfer aborted. */
    rb_define_const(rb_mFtpStatus, "TRANSFER_ABORTED", INT2NUM(sfFtpTransferAborted));
    /* 450: requested file action not taken. */
    rb_define_const(rb_mFtpStatus, "FILE_ACTION_ABORTED", INT2NUM(sfFtpFileActionAborted));
    /* 451: requested action aborted, local error in processing. */
    rb_define_const(rb_mFtpStatus, "LOCAL_ERROR", INT2NUM(sfFtpLocalError));
    /* 452: requested action not taken; insufficient storage space in system, file unavailable. */
    rb_define_const(rb_mFtpStatus, "INSUFFICIENT_STORAGE_SPACE",
                    INT2NUM(sfFtpInsufficientStorageSpace));
    /* 500: syntax error, command unrecognized. */
    rb_define_const(rb_mFtpStatus, "COMMAND_UNKNOWN", INT2NUM(sfFtpCommandUnknown));
    /* 501: syntax error in parameters or arguments. */
    rb_define_const(rb_mFtpStatus, "PARAMETERS_UNKNOWN", INT2NUM(sfFtpParametersUnknown));
    /* 502: command not implemented. */
    rb_define_const(rb_mFtpStatus, "COMMAND_NOT_IMPLEMENTED", INT2NUM(sfFtpCommandNotImplemented));
    /* 503: bad sequence of commands. */
    rb_define_const(rb_mFtpStatus, "BAD_COMMAND_SEQUENCE", INT2NUM(sfFtpBadCommandSequence));
    /* 504: command not implemented for that parameter. */
    rb_define_const(rb_mFtpStatus, "PARAMETER_NOT_IMPLEMENTED",
                    INT2NUM(sfFtpParameterNotImplemented));
    /* 530: not logged in. */
    rb_define_const(rb_mFtpStatus, "NOT_LOGGED_IN", INT2NUM(sfFtpNotLoggedIn));
    /* 532: account needed for storing files. */
    rb_define_const(rb_mFtpStatus, "NEED_ACCOUNT_TO_STORE", INT2NUM(sfFtpNeedAccountToStore));
    /* 550: requested action not taken, file unavailable. */
    rb_define_const(rb_mFtpStatus, "FILE_UNAVAILABLE", INT2NUM(sfFtpFileUnavailable));
    /* 551: requested action aborted, page type unknown. */
    rb_define_const(rb_mFtpStatus, "PAGE_TYPE_UNKNOWN", INT2NUM(sfFtpPageTypeUnknown));
    /* 552: requested file action aborted, exceeded storage allocation. */
    rb_define_const(rb_mFtpStatus, "NOT_ENOUGH_MEMORY", INT2NUM(sfFtpNotEnoughMemory));
    /* 553: requested action not taken, file name not allowed. */
    rb_define_const(rb_mFtpStatus, "FILENAME_NOT_ALLOWED", INT2NUM(sfFtpFilenameNotAllowed));
    /* 1000 (SFML-specific): the response is not a valid FTP one. */
    rb_define_const(rb_mFtpStatus, "INVALID_RESPONSE", INT2NUM(sfFtpInvalidResponse));
    /* 1001 (SFML-specific): the connection with the server failed. */
    rb_define_const(rb_mFtpStatus, "CONNECTION_FAILED", INT2NUM(sfFtpConnectionFailed));
    /* 1002 (SFML-specific): the connection with the server closed. */
    rb_define_const(rb_mFtpStatus, "CONNECTION_CLOSED", INT2NUM(sfFtpConnectionClosed));
    /* 1003 (SFML-specific): invalid file for upload/download. */
    rb_define_const(rb_mFtpStatus, "INVALID_FILE", INT2NUM(sfFtpInvalidFile));
}
