#ifndef VIPSER_SNIFFING_H
#define VIPSER_SNIFFING_H
#include <sys/types.h>

#include "types.h"

// Initialise the libmagic interface
RESULT magic_init();

// Determine the name (e.g. webp) of the file which is opened with the `fd`
// descriptor.  This operation will move the file pointer forward and will
// not seek to the beginning of the file.  This function will attempt to
// look at magic bytes before falling back to using libmagic to do a full
// determination
RESULT determine_descriptor_type(int fd, char **name);

// Determine the image type at the pointed to buffer
RESULT determine_buffer_type(Buffer *input, format_t *fmt);

// Map a format name to the corresponding format_t
format_t format_from_name(const char *name);

// Return the name string (e.g. jpeg) of a format
char *get_format_name(format_t fmt);

#endif // VIPSER_SNIFFING_H
