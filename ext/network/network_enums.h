#ifndef SFML_RB_NETWORK_NETWORK_ENUMS_H
#define SFML_RB_NETWORK_NETWORK_ENUMS_H

#include <ruby.h>

#include "core/sfml.h"

void Init_NetworkEnums(VALUE rb_module);

const char *socket_status_name(sfSocketStatus status);

sfSocketStatus socket_status_from_rb(VALUE rb_status);

const char *http_method_name(sfHttpMethod method);

sfHttpMethod http_method_from_rb(VALUE rb_method);

const char *http_status_name(sfHttpStatus status);

const char *ftp_transfer_mode_name(sfFtpTransferMode mode);

sfFtpTransferMode ftp_transfer_mode_from_rb(VALUE rb_mode);

const char *ftp_status_name(sfFtpStatus status);

#endif //SFML_RB_NETWORK_NETWORK_ENUMS_H
