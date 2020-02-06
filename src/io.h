#ifndef VIPSER_IO_H
#define VIPSER_IO_H

#include <stddef.h>

#include "types.h"

#define BUFFER_SIZE 4096
RESULT read_all(int input_fd, Buffer *output);

typedef unsigned level;
#define DEBUG 1u
#define INFO 2u
#define WARN 4u
#define ERROR 8u

void v_log_init();
void v_log_errno(int err, const char *fmt, ...);
void v_vips_err(const char *fmt, ...);
void v_log(level lvl, const char *fmt, ...);

#endif // VIPSER_IO_H
