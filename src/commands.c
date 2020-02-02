//
// Created by jhford on 2/2/20.
//

#include <strings.h>
#include <sys/errno.h>
#include <unistd.h> // only for syncfs()

#include <vips/vips.h>

#include "commands.h"
#include "io.h"
#include "sniffing.h"
#include "types.h"
#include "vips_glue.h"

RESULT parse_int(char *val, int *out) {
  errno = 0;

  long long_v = strtol(val, NULL, 0);

  if (errno != 0) {
    v_log_errno(ERROR, errno);
    return FAIL;
  }

  if (long_v > INT_MAX) {
    v_log(ERROR, "parse_int: %s is larger than allowed max of %d", val,
          INT_MAX);
    return FAIL;
  }

  *out = (int)long_v;

  v_log(DEBUG, "parsed %s to int %d", val, *out);
  return OK;
}

RESULT parse_dbl(char *val, double *out) {
  errno = 0;

  *out = strtod(val, NULL);

  if (errno != 0) {
    v_log_errno(ERROR, errno);
    *out = 0;
    return FAIL;
  }

  v_log(DEBUG, "parsed %s to dbl %f", val, *out);
  return OK;
}

// Call once to get argc (pass NULL argv), then an allocated
// buffer as argv to get the strings.  NOTE: this function will
// modify the contents of cmd, so it is important that any memory
// passed to cmd which shouldn't be modified is copied first
int get_command_args(char *cmd, char **argv) {
  char *sep = ",";
  char *s = NULL;

  int argc = 0;
  for (char *token = strtok_r(cmd, sep, &s); token;
       token = strtok_r(NULL, sep, &s)) {
    if (NULL != argv) {
      argv[argc] = token;
    }
    argc += 1;
  }

  return argc;
}

bool_t is_resize_command(int argc, char **argv) {
  if (argc != 3) {
    return V_FALSE;
  }

  if (0 == strncmp(RESIZE_CMD, argv[0], strlen(RESIZE_CMD))) {
    return V_TRUE;
  } else if (0 == strncmp(STRETCH_CMD, argv[0], strlen(STRETCH_CMD))) {
    return V_TRUE;
  } else if (0 == strncmp(EXPAND_CMD, argv[0], strlen(EXPAND_CMD))) {
    return V_TRUE;
  }

  return V_FALSE;
}

RESULT resize_command(VipsImage *in, VipsImage **out, int argc, char **argv) {
  int width;
  int height;

  if (OK != parse_int(argv[1], &width)) {
    v_log(ERROR, "failed to parse resize width");
    return FAIL;
  }
  if (OK != parse_int(argv[2], &height)) {
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

  printf("%d\n", opts);

  return resize_image(in, out, height, width, opts);
}

bool_t is_extract_command(int argc, char **argv) {
  if (argc == 5 && 0 == strncmp(EXTRACT_CMD, argv[0], strlen(EXPAND_CMD))) {
    return V_TRUE;
  }

  return V_FALSE;
}

RESULT extract_command(VipsImage *in, VipsImage **out, int argc, char **argv) {
  int left;
  int top;
  int width;
  int height;

  if (OK != parse_int(argv[1], &left)) {
    v_log(ERROR, "failed to parse extract left");
    return FAIL;
  }
  if (OK != parse_int(argv[2], &top)) {
    v_log(ERROR, "failed to parse extract top");
    return FAIL;
  }
  if (OK != parse_int(argv[3], &width)) {
    v_log(ERROR, "failed to parse extract width");
    return FAIL;
  }
  if (OK != parse_int(argv[4], &height)) {
    v_log(ERROR, "failed to parse extract height");
    return FAIL;
  }

  return extract_image(in, out, left, top, width, height);
}

bool_t is_autorotate_command(int argc, char **argv) {
  if (argc == 1 && 0 == strncmp(AUTOROT_CMD, argv[0], strlen(AUTOROT_CMD))) {
    return V_TRUE;
  }
  return V_FALSE;
}

RESULT autorotate_command(VipsImage *in, VipsImage **out, int argc,
                          char **argv) {
  return autorot_image(in, out);
}

bool_t is_rotate_command(int argc, char **argv) {
  if (argc == 2 && 0 == strncmp(ROTATE_CMD, argv[0], strlen(ROTATE_CMD))) {
    return V_TRUE;
  }

  return V_FALSE;
}

RESULT rotate_command(VipsImage *in, VipsImage **out, int argc, char **argv) {
  int rotation;
  if (OK != parse_int(argv[1], &rotation)) {
    v_log(ERROR, "failed to parse rotation");
    return FAIL;
  }

  if (OK != rotate_image(in, out, rotation)) {
    v_log(ERROR, "rotate %d", rotation);
    return FAIL;
  }

  return OK;
}

bool_t is_blur_command(int argc, char **argv) {
  if (argc == 2 && 0 == strncmp(BLUR_CMD, argv[0], strlen(BLUR_CMD))) {
    return V_TRUE;
  }

  return V_FALSE;
}

RESULT blur_command(VipsImage *in, VipsImage **out, int argc, char **argv) {
  double sigma;
  if (OK != parse_dbl(argv[1], &sigma)) {
    v_log(ERROR, "failed to parse gaussian blur sigma");
    return FAIL;
  }
  if (OK != gaussblur_image(in, out, sigma)) {
    v_log(ERROR, "blur %f", sigma);
    return FAIL;
  }

  return OK;
}

bool_t is_quality_command(int argc, char **argv) {
  if (argc == 2 && 0 == strncmp(QUALITY_CMD, argv[0], strlen(QUALITY_CMD))) {
    return V_TRUE;
  }

  return V_FALSE;
}

RESULT quality_command(int *quality, int argc, char **argv) {
  if (OK != parse_int(argv[1], quality)) {
    *quality = DEFAULT_QUALITY;
    v_log(ERROR, "failed to parse quality");
    return FAIL;
  }

  v_log(INFO, "export %d", quality);
  return OK;
}

bool_t is_export_command(int argc, char **argv) {
  if (argc == 2 && 0 == strncmp(EXPORT_CMD, argv[0], strlen(EXPORT_CMD))) {
    return V_TRUE;
  }

  return V_FALSE;
}

RESULT export_command(format_t *format, int argc, char **argv) {
  *format = format_from_name(argv[1]);
  if (*format == UNK) {
    v_log(ERROR, "unknown export format");
    return FAIL;
  }

  v_log(INFO, "export %s", get_format_name(*format));
  return OK;
}

char *cpystr(char *in) {
  char *buf = NULL;
  size_t n = strlen(in);
  buf = malloc(sizeof(char *) * (n + 1));
  memcpy(buf, in, n);
  buf[n] = 0;
  return buf;
}

RESULT run_command(VipsImage **in, format_t *format, int *quality, char *cmd) {
  VipsImage *out = NULL;

  v_log(DEBUG, "parsing command arguments");

  // Copy the command string because it's edited
  char *cmd_copy = cpystr(cmd);
  int argc = get_command_args(cmd_copy, NULL);
  free(cmd_copy);

  char **argv = malloc(sizeof(char *) * argc);
  memset(argv, 0, sizeof(char *) * argc);

  // is used while processing the strings
  cmd_copy = cpystr(cmd);
  get_command_args(cmd_copy, argv);

  // This must be freed at the end of the function as the memory
  bool_t edited = V_TRUE;
  RESULT outcome = FAIL;

  if (is_resize_command(argc, argv)) {
    outcome = resize_command(*in, &out, argc, argv);
  } else if (is_extract_command(argc, argv)) {
    outcome = extract_command(*in, &out, argc, argv);
  } else if (is_blur_command(argc, argv)) {
    outcome = blur_command(*in, &out, argc, argv);
  } else if (is_rotate_command(argc, argv)) {
    outcome = rotate_command(*in, &out, argc, argv);
  } else if (is_autorotate_command(argc, argv)) {
    outcome = autorotate_command(*in, &out, argc, argv);
  } else if (is_export_command(argc, argv)) {
    edited = V_FALSE;
    outcome = export_command(format, argc, argv);
  } else if (is_quality_command(argc, argv)) {
    edited = V_FALSE;
    outcome = quality_command(quality, argc, argv);
  } else {
    edited = V_FALSE;
    outcome = FAIL;
  }

  if (edited && OK == outcome) {
    g_object_unref(*in);
    *in = out;
    v_log(INFO, "image mutation, swapping");
  }

  free(cmd_copy);
  free(argv);

  return outcome;
}
