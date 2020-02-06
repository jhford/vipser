//
// Created by jhford on 2/2/20.
//

#ifndef VIPSER_COMMANDS_H
#define VIPSER_COMMANDS_H
#include "types.h"

#define DEFAULT_QUALITY 75

// A command is a string in the format "CMDNAME,ARG1,ARG2" and specifies which
// operation should be applied on the VipsImage pointed to by `in` and then sent
// back as VipsImage `out`.  The format_t pointed to will be written to with an
// image format change specified by a command.  Similarly, the integer pointed
// to by quality will be written to with the integer (0-100) quality specified
// by a command.  The integer pointed to by edited will be truthy (1) if the
// command has created a new `out` which is different to `in`.  The `cmd` string
// is the raw command, for example 'RESIZE,100,100'. The returned image `out`
// must be released with g_object_unref()
RESULT run_command(VipsImage **in, format_t *format, int *quality, char *cmd);

// Run all commands from the program's argument list using the input
// buffer and store the results in the output buffer.  The format provided
// is the format of the image file written to the output buffer.  The quality
// is an integer in the 0-100 range which is passed to the image encoder to
// specify the quality desired.  Both format and quality are passed as pointers
// because the might be modified by a command
RESULT run_commands(int argc, char **argv, Buffer input, Buffer *output, format_t *format, int *quality);

// Define the strings used for the various commands
#define AUTOROT_CMD "AUTOROT"
#define ROTATE_CMD "ROTATE"
#define RESIZE_CMD "RESIZE"
#define STRETCH_CMD "STRETCH"
#define EXPAND_CMD "EXPAND"
#define EXTRACT_CMD "EXTRACT"
#define EMBED_BLACK_CMD "EMBBLK"
#define EMBED_WHITE_CMD "EMBWHT"
#define BLUR_CMD "BLUR"
#define EXPORT_CMD "EXPORT"
#define QUALITY_CMD "QUALITY"

#endif // VIPSER_COMMANDS_H
