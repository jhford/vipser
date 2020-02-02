#ifndef VIPSER_SNIFFING_H
#define VIPSER_SNIFFING_H
#include <sys/types.h>

#include "types.h"

// Initialise the libmagic interface
RESULT magic_init();

// Determine the name (e.g. webp) of the file which is opened with the `fd` descriptor
RESULT determine_descriptor_type(int fd, char **name);

// Determine the format_t of a given buffer
RESULT determine_buffer_type(size_t, char *buf, format_t *fmt);

// Map a format name to the corresponding format_t
format_t format_from_name(const char* name);

char* get_format_name(format_t fmt);

#endif //VIPSER_SNIFFING_H
