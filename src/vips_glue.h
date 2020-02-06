#ifndef VIPSER_VIPS_GLUE_H
#define VIPSER_VIPS_GLUE_H
#include "types.h"

RESULT init_vips(char* argv0);

RESULT read_input(int fd, void **buf, size_t *len, format_t *format);
RESULT import_image(void *buf, size_t n, bool_t seq, VipsImage **out);

RESULT rotate_image(VipsImage *in, VipsImage **out, int deg);
RESULT autorot_image(VipsImage *in, VipsImage **out);
RESULT extract_image(VipsImage *in, VipsImage **out, int left, int top, int width, int height);
RESULT embed_image(VipsImage *in, VipsImage **out, int x, int y, int width, int height, VipsExtend extend);
RESULT gaussblur_image(VipsImage *in, VipsImage **out, double sigma);
RESULT resize_image(VipsImage *in, VipsImage **out, int newHeight, int newWidth, unsigned opts);
RESULT export_image(VipsImage *in, void **out, size_t *n, format_t format, int quality);

#endif // VIPSER_VIPS_GLUE_H
