//
// Created by jhford on 2/2/20.
//

#ifndef VIPSER_COMMANDS_H
#define VIPSER_COMMANDS_H


#define DEFAULT_QUALITY 75

// A command is a string in the format "CMDNAME,ARG1,ARG2" and specifies which operation should
// be applied on the VipsImage pointed to by `in` and then sent back as VipsImage `out`.  The
// format_t pointed to will be written to with an image format change specified by a command.  Similarly,
// the integer pointed to by quality will be written to with the integer (0-100) quality specified
// by a command.  The integer pointed to by edited will be truthy (1) if the command has created
// a new `out` which is different to `in`.  The `cmd` string is the raw command, for example 'RESIZE,100,100'.
// The returned image `out` must be released with g_object_unref()
RESULT run_command(VipsImage **in, format_t* format, int* quality, char* cmd);

#define AUTOROT_CMD "AUTOROT"
#define ROTATE_CMD "ROTATE"
#define RESIZE_CMD "RESIZE"
#define STRETCH_CMD "STRETCH"
#define EXPAND_CMD "EXPAND"
#define EXTRACT_CMD "EXTRACT"
#define BLUR_CMD "BLUR"
#define EXPORT_CMD "EXPORT"
#define QUALITY_CMD "QUALITY"

#endif //VIPSER_COMMANDS_H
