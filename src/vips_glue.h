#ifndef VIPSER_VIPS_GLUE_H
#define VIPSER_VIPS_GLUE_H
#include "types.h"

// Initialize the vips library
RESULT init_vips(char *argv0);

// Read all input on the file descriptor and set in output, along with the
// format of the data in the output buffer
RESULT read_input(int input_fd, Buffer *output, format_t *format);

// Import image data in the input buffer into a VipsImage pointed to by
// output.  If the seq parameter is true, use the vips sequential access
// mode.  This is faster and uses less memory but cannot be used with
// rotations or warps the image.
RESULT import_image(bool_t seq, Buffer input, VipsImage **output);

// Rotate an image by deg degrees.  Only multiples of 90 are allowed, other
// values will result in a failure
RESULT rotate_image(VipsImage *input, VipsImage **output, int deg);

// Rotate an image based on any rotation headers contained in the metadata
RESULT autorot_image(VipsImage *input, VipsImage **output);

// Extract a section of an image
RESULT extract_image(VipsImage *input, VipsImage **output, int left, int top, int width, int height);

// Embed an image inside of a different size container, using the extend option to determine
// how new pixels are generated
RESULT embed_image(VipsImage *input, VipsImage **output, int x, int y, int width, int height, VipsExtend extend);

// Perform a gaussian blur on an image
RESULT gaussblur_image(VipsImage *input, VipsImage **output, double sigma);

// Resize an image using a given sizing strategy
RESULT resize_image(VipsImage *input, VipsImage **output, int height, int width, unsigned opts);

// Export an image into the output buffer using the provided format and quality
RESULT export_image(VipsImage *input, Buffer *output, format_t format, int quality);

#endif // VIPSER_VIPS_GLUE_H
