//
// Created by jhford on 2/2/20.
//

#include <strings.h>
#include <sys/errno.h>

#include <vips/vips.h>

#include "commands.h"
#include "io.h"
#include "sniffing.h"
#include "types.h"
#include "vips_glue.h"

// NOTE: This function should only be used on strings which are known to be
// null terminated, like those given by argv
char *cpystr(char *in) {
    char *buf = NULL;
    size_t n = strlen(in);
    buf = malloc(sizeof(char *) * (n + 1));
    if (buf == NULL) {
        v_log(DEBUG, "failed to allocate buffer, exiting");
        exit(EXIT_FAILURE);
    }
    memcpy(buf, in, n);
    buf[n] = 0;
    return buf;
}

RESULT parse_int(char *val, int *out) {
    // Parse the integer
    errno = 0;
    long long_v = strtol(val, NULL, 0);
    if (errno != 0) {
        v_log_errno(errno, "parsing int from %s", val);
        return FAIL;
    }

    // If the long is too large for an int, it's an error
    if (long_v > INT_MAX) {
        v_log(ERROR, "parse_int: %s is larger than allowed max of %d", val, INT_MAX);
        return FAIL;
    }

    // Store the integer
    *out = (int)long_v;

    v_log(DEBUG, "parsed %s to int %d", val, *out);
    return OK;
}

RESULT parse_dbl(char *val, double *out) {
    // Parse the float
    errno = 0;
    *out = strtod(val, NULL);
    if (errno != 0) {
        v_log_errno(errno, "parsing double from %s", val);
        *out = 0;
        return FAIL;
    }

    v_log(DEBUG, "parsed %s to dbl %f", val, *out);
    return OK;
}

// Call once to get argc (pass NULL argv), then an allocated
// buffer as argv to get the strings.
int get_command_args(char *cmd, char **argv) {
    char *sep = ",";
    char *s = NULL;

    // Copy the string so that it's safe to tokenize
    char *copy = cpystr(cmd);

    // Each call to strtok_r returns the next token.  This version of
    // of the call uses a pointer so that it is re-entrant
    int argc = 0;
    for (char *token = strtok_r(cmd, sep, &s); token; token = strtok_r(NULL, sep, &s)) {
        if (NULL != argv) {
            argv[argc] = token;
        }
        argc += 1;
    }

    // Release the memory of the temporary copy
    free(copy);

    return argc;
}

bool_t is_resize_command(int argc, char **argv) {
    if (argc != 3) {
        return false;
    }

    if (0 == strncmp(RESIZE_CMD, argv[0], strlen(RESIZE_CMD))) {
        return true;
    } else if (0 == strncmp(STRETCH_CMD, argv[0], strlen(STRETCH_CMD))) {
        return true;
    } else if (0 == strncmp(EXPAND_CMD, argv[0], strlen(EXPAND_CMD))) {
        return true;
    }

    return false;
}

RESULT resize_command(VipsImage *in, VipsImage **out, char **argv) {
    int width = 0;
    int height = 0;

    if (parse_int(argv[1], &width)) {
        v_log(ERROR, "failed to parse resize width");
        return FAIL;
    }
    if (parse_int(argv[2], &height)) {
        v_log(ERROR, "failed to parse resize height");
        return FAIL;
    }

    unsigned opts = 0;
    if (0 == strncmp(RESIZE_CMD, argv[0], strlen(RESIZE_CMD))) {
        opts = RESIZE;
    } else if (0 == strncmp(EXPAND_CMD, argv[0], strlen(EXPAND_CMD))) {
        opts = EXPAND;
    } else if (0 == strncmp(STRETCH_CMD, argv[0], strlen(STRETCH_CMD))) {
        opts = STRETCH;
    }

    return resize_image(in, out, height, width, opts);
}

bool_t is_extract_command(int argc, char **argv) {
    if (argc == 5 && 0 == strncmp(EXTRACT_CMD, argv[0], strlen(EXPAND_CMD))) {
        return true;
    }

    return false;
}

RESULT extract_command(VipsImage *in, VipsImage **out, char **argv) {
    int left = 0;
    int top = 0;
    int width = 0;
    int height = 0;

    if (parse_int(argv[1], &left)) {
        v_log(ERROR, "failed to parse extract left");
        return FAIL;
    }
    if (parse_int(argv[2], &top)) {
        v_log(ERROR, "failed to parse extract top");
        return FAIL;
    }
    if (parse_int(argv[3], &width)) {
        v_log(ERROR, "failed to parse extract width");
        return FAIL;
    }
    if (parse_int(argv[4], &height)) {
        v_log(ERROR, "failed to parse extract height");
        return FAIL;
    }

    return extract_image(in, out, left, top, width, height);
}

bool_t is_embed_command(int argc, char **argv) {
    if (argc != 5) {
        return false;
    }

    if (0 == strncmp(EMBED_BLACK_CMD, argv[0], strlen(EMBED_BLACK_CMD))) {
        return true;
    }

    if (0 == strncmp(EMBED_WHITE_CMD, argv[0], strlen(EMBED_WHITE_CMD))) {
        return true;
    }

    return false;
}

RESULT embed_command(VipsImage *in, VipsImage **out, char **argv) {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    if (parse_int(argv[1], &x)) {
        v_log(ERROR, "failed to parse extract x");
        return FAIL;
    }
    if (parse_int(argv[2], &y)) {
        v_log(ERROR, "failed to parse extract y");
        return FAIL;
    }
    if (parse_int(argv[3], &width)) {
        v_log(ERROR, "failed to parse extract width");
        return FAIL;
    }
    if (parse_int(argv[4], &height)) {
        v_log(ERROR, "failed to parse extract height");
        return FAIL;
    }

    VipsExtend extend = VIPS_EXTEND_WHITE;

    if (0 == strncmp(EMBED_BLACK_CMD, argv[0], strlen(EMBED_BLACK_CMD))) {
        extend = VIPS_EXTEND_BLACK;
    } else if (0 == strncmp(EMBED_WHITE_CMD, argv[0], strlen(EMBED_WHITE_CMD))) {
        extend = VIPS_EXTEND_WHITE;
    }

    return embed_image(in, out, x, y, width, height, extend);
}

bool_t is_autorotate_command(int argc, char **argv) {
    if (argc == 1 && 0 == strncmp(AUTOROT_CMD, argv[0], strlen(AUTOROT_CMD))) {
        return true;
    }
    return false;
}

RESULT autorotate_command(VipsImage *in, VipsImage **out) { return autorot_image(in, out); }

bool_t is_rotate_command(int argc, char **argv) {
    if (argc == 2 && 0 == strncmp(ROTATE_CMD, argv[0], strlen(ROTATE_CMD))) {
        return true;
    }

    return false;
}

RESULT rotate_command(VipsImage *in, VipsImage **out, char **argv) {
    int rotation = 0;
    if (parse_int(argv[1], &rotation)) {
        v_log(ERROR, "failed to parse rotation");
        return FAIL;
    }

    if (rotate_image(in, out, rotation)) {
        v_log(ERROR, "rotate %d", rotation);
        return FAIL;
    }

    return OK;
}

bool_t is_blur_command(int argc, char **argv) {
    if (argc == 2 && 0 == strncmp(BLUR_CMD, argv[0], strlen(BLUR_CMD))) {
        return true;
    }

    return false;
}

RESULT blur_command(VipsImage *in, VipsImage **out, char **argv) {
    double sigma = 0;

    if (parse_dbl(argv[1], &sigma)) {
        v_log(ERROR, "failed to parse gaussian blur sigma");
        return FAIL;
    }

    if (gaussblur_image(in, out, sigma)) {
        v_log(ERROR, "blur %f", sigma);
        return FAIL;
    }

    return OK;
}

bool_t is_quality_command(int argc, char **argv) {
    if (argc == 2 && 0 == strncmp(QUALITY_CMD, argv[0], strlen(QUALITY_CMD))) {
        return true;
    }

    return false;
}

RESULT quality_command(int *quality, char **argv) {
    if (parse_int(argv[1], quality)) {
        *quality = DEFAULT_QUALITY;
        v_log(ERROR, "failed to parse quality");
        return FAIL;
    }

    v_log(INFO, "export %d", quality);
    return OK;
}

bool_t is_export_command(int argc, char **argv) {
    if (argc == 2 && 0 == strncmp(EXPORT_CMD, argv[0], strlen(EXPORT_CMD))) {
        return true;
    }

    return false;
}

RESULT export_command(format_t *format, char **argv) {
    *format = format_from_name(argv[1]);
    if (*format == UNK) {
        v_log(ERROR, "unknown export format");
        return FAIL;
    }

    v_log(INFO, "export %s", get_format_name(*format));
    return OK;
}


// Given command (e.g. RESIZE,1,2), split the command into the composite
// parts (string RESIZE, int 1 and int 2) then pass those to the various
// is-a-command functions to determine which command it is.  The first
// command which returns true has its matching command executed.  The
// function command will assume that the argument list is valid for that
// command.  The argument list for the command RESIZE,1,2 will be
// ["REISIZE\0", "1\0", "2\0"].
// The passed-in VipsImage reference will need to be unref'd by the caller
RESULT run_command(VipsImage **in, format_t *format, int *quality, char *cmd) {
    VipsImage *out = NULL;

    v_log(DEBUG, "parsing command arguments");

    // Determine the number of command arguments
    int argc = get_command_args(cmd, NULL);

    // Allocate a list of pointers to the arguments in the
    // argument list
    char **argv = malloc(sizeof(char *) * argc);
    if (NULL == argv) {
        v_log(DEBUG, "failed to allocate buffer, exiting");
        exit(EXIT_FAILURE);
    }
    memset(argv, 0, sizeof(char *) * argc);

    // Set the command arguments
    get_command_args(cmd, argv);

    // Store whether or not the command edited the image
    bool_t edited = true;

    // Store the outcome of the command, rather than early
    // return so that the needed cleanup can happen
    RESULT outcome = FAIL;

    if (is_resize_command(argc, argv)) {
        outcome = resize_command(*in, &out, argv);
    } else if (is_embed_command(argc, argv)) {
        outcome = embed_command(*in, &out, argv);
    } else if (is_extract_command(argc, argv)) {
        outcome = extract_command(*in, &out, argv);
    } else if (is_blur_command(argc, argv)) {
        outcome = blur_command(*in, &out, argv);
    } else if (is_rotate_command(argc, argv)) {
        outcome = rotate_command(*in, &out, argv);
    } else if (is_autorotate_command(argc, argv)) {
        outcome = autorotate_command(*in, &out);
    } else if (is_export_command(argc, argv)) {
        edited = false;
        outcome = export_command(format, argv);
    } else if (is_quality_command(argc, argv)) {
        edited = false;
        outcome = quality_command(quality, argv);
    } else {
        edited = false;
        if (argc > 0) {
            v_log(ERROR, "unknown command: %s", argv[0]);
        } else {
            v_log(ERROR, "unspecified command");
        }
        outcome = FAIL;
    }

    // When an operation has successfully applied, we want to swap the input
    // for the output of the command and unref the original input so as not
    // to leak memory
    if (edited && OK == outcome) {
        g_object_unref(*in);
        *in = out;
        v_log(INFO, "image mutation, swapping");
    }

    // Release the memory allocated for the pointers to each command argument
    free(argv);

    return outcome;
}

// Argv[] needs to have 'something' at index 0 to account for this
// function being designed to consume an argv as given by the system
RESULT run_commands(int argc, char **argv, Buffer input, Buffer *output, format_t *format, int *quality) {

    // VipsImage that's being processed
    VipsImage *image = NULL;

    // We need to track if the commands failed so that we can skip
    // exporting the image
    int outcome = OK;

    // Import the image
    if (import_image(false, input, &image)) {
        v_log(ERROR, "importing image");
        return FAIL;
    }
    v_log(INFO, "image imported");

    // Run each command specified
    for (int i = 1; i < argc; i++) {
        v_log(INFO, "initiating command %s", argv[i]);

        if (run_command(&image, format, quality, argv[i])) {
            v_log(ERROR, "error running command: %s", argv[i]);
            outcome = FAIL;
            break;
        }
    }

    // If a command failed, release the VipsImage and return failing
    if (outcome) {
        g_object_unref(image);
        return FAIL;
    }

    // If the image was successfully processed, export it
    if (export_image(image, output, *format, *quality)) {
        v_log(ERROR, "exporting image");
        return FAIL;
    }

    // Unref the in-flight image to avoid leaking
    g_object_unref(image);

    return OK;
}
