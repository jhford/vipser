#ifndef VIPSER_VIPS_GLUE_H
#define VIPSER_VIPS_GLUE_H
#include "types.h"

#define STRETCH 1u
#define EXPAND  2u
#define DEFAULT_QUALITY 75

// A command is a string in the format "CMDNAME,ARG1,ARG2" and specifies which operation should
// be applied on the VipsImage pointed to by `in` and then sent back as VipsImage `out`.  The
// format_t pointed to will be written to with an image format change specified by a command.  Similarly,
// the integer pointed to by quality will be written to with the integer (0-100) quality specified
// by a command.  The integer pointed to by edited will be truthy (1) if the command has created
// a new `out` which is different to `in`.  The `cmd` string is the raw command, for example 'RESIZE,100,100'.
// The returned image `out` must be released with g_object_unref()
RESULT run_command(VipsImage *in, VipsImage **out, format_t* format, int* quality, int* edited, char* cmd);

// Export an image into a buffer in the specified format (e.g. JPEG) at a specified quality (e.g. 75).  This
// buffer is allocated by Vips internally and needs to be freed with g_free();
RESULT export_image(VipsImage* in, void** out, size_t *n, format_t format, int quality);

#endif //VIPSER_VIPS_GLUE_H
