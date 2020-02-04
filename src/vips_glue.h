#ifndef VIPSER_VIPS_GLUE_H
#define VIPSER_VIPS_GLUE_H
#include "types.h"

RESULT rotate_image(VipsImage *in, VipsImage **out, int deg);
RESULT autorot_image(VipsImage *in, VipsImage **out);
RESULT extract_image(VipsImage *in, VipsImage **out, int left, int top, int width, int height);
RESULT embed_image_white(VipsImage *in, VipsImage **out, int x, int y, int width, int height);
RESULT embed_image_black(VipsImage *in, VipsImage **out, int x, int y, int width, int height);
RESULT embed_image(VipsImage *in, VipsImage **out, int x, int y, int width, int height, VipsExtend extend);
RESULT gaussblur_image(VipsImage *in, VipsImage **out, double sigma);
RESULT resize_image(VipsImage *in, VipsImage **out, int newHeight, int newWidth, unsigned opts);
RESULT export_image(VipsImage *in, void **out, size_t *n, format_t format, int quality);

#endif // VIPSER_VIPS_GLUE_H
