#ifndef SFML_RB_NETWORK_FTP_H
#define SFML_RB_NETWORK_FTP_H

#include <ruby.h>

#include "core/sfml.h"

void Init_Ftp(VALUE rb_module);

VALUE Get_Klass_Ftp(void);
VALUE Get_Klass_FtpResponse(void);
VALUE Get_Klass_FtpDirectoryResponse(void);
VALUE Get_Klass_FtpListingResponse(void);

void *Get_Ftp_Struct(VALUE self);
void *Get_FtpResponse_Struct(VALUE self);
void *Get_FtpDirectoryResponse_Struct(VALUE self);
void *Get_FtpListingResponse_Struct(VALUE self);

#endif //SFML_RB_NETWORK_FTP_H
