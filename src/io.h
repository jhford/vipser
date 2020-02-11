
#ifndef VIPSER_IO_H
#define VIPSER_IO_H

// This enables the GNU extension needed to use [v]asprintf
#define _GNU_SOURCE

#include <stddef.h>

#include "types.h"

// For operations which require a buffer for reading or writing, this
// is the size of buffer.
#define BUFFER_SIZE 4096
RESULT read_all(int input_fd, Buffer *output);

// Defined logging levels
typedef unsigned level;
#define DEBUG 1u
#define INFO 2u
#define WARN 4u
#define ERROR 8u

// Initialize logging.  This function should be called before any other
// logging functions if a correct program time is desired
void v_log_init();

// Log an error with an associated system errno
void v_log_errno(int err, const char *fmt, ...);

// Log the most recent error message from libvips
void v_vips_err(const char *fmt, ...);

// Log a message at a level
void v_log(level lvl, const char *fmt, ...);

#endif // VIPSER_IO_H
