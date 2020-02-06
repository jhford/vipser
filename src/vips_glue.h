#ifndef VIPSER_VIPS_GLUE_H
#define VIPSER_VIPS_GLUE_H
#include "types.h"

RESULT init_vips(char *argv0);

RESULT read_input(int input_fd, Buffer *output, format_t *format);
RESULT import_image(bool_t seq, Buffer input, VipsImage **output);

RESULT rotate_image(VipsImage *input, VipsImage **output, int deg);
RESULT autorot_image(VipsImage *input, VipsImage **output);
RESULT extract_image(VipsImage *input, VipsImage **output, int left, int top, int width, int height);
RESULT embed_image(VipsImage *input, VipsImage **output, int x, int y, int width, int height, VipsExtend extend);
RESULT gaussblur_image(VipsImage *input, VipsImage **output, double sigma);
RESULT resize_image(VipsImage *input, VipsImage **output, int height, int width, unsigned opts);
RESULT export_image(VipsImage *input, Buffer *output, format_t format, int quality);

#endif // VIPSER_VIPS_GLUE_H
