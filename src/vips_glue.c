#include <stdio.h>
#include <vips/vips.h>

#include "io.h"
#include "sniffing.h"
#include "vips_glue.h"

RESULT init_vips(char *argv0) {
    if (VIPS_INIT(argv0)) {
        v_vips_err("initialising vips");
        return FAIL;
    }
    return OK;
}

RESULT read_input(int input_fd, Buffer *output, format_t *format) {
    // Read the full input into memory
    if (OK != read_all(input_fd, output)) {
        v_log(ERROR, "error while reading input");
        return FAIL;
    }

    // Determine the type of input buffer
    if (OK != determine_buffer_type(output, format)) {
        v_log(ERROR, "error determining buffer type");
        return FAIL;
    }

    // Only load known image types
    if (UNK == *format) {
        char *x = (char *)output->data;
        if (output->len > 5) {
            v_log(ERROR, "unsupported image type [0x%x 0x%x 0x%x 0x%x...]", x[0], x[1], x[2], x[3]);
        } else if (output->len == 4) {
            v_log(ERROR, "unsupported image type [0x%x 0x%x 0x%x 0x%x]", x[0], x[1], x[2], x[3]);
        } else {
            // Likely not an image
            v_log(ERROR, "unsupported image type, input less than 4 bytes");
        }
        return FAIL;
    }

    return OK;
}

RESULT import_image(bool_t seq, Buffer input, VipsImage **output) {
    if (seq) {
        *output = vips_image_new_from_buffer(input.data, input.len, "access", VIPS_ACCESS_SEQUENTIAL, NULL);
    } else {
        *output = vips_image_new_from_buffer(input.data, input.len, "", NULL);
    }
    if (*output == NULL) {
        v_vips_err("error creating VipsImage from buffer");
        return FAIL;
    }
    return OK;
}

RESULT rotate_image(VipsImage *input, VipsImage **output, int deg) {
    v_log(INFO, "rotate %3d degree", deg);
    VipsAngle angle;
    switch (deg % 360) {
    case 0:
        angle = VIPS_ANGLE_D0;
        break;
    case 90:
        angle = VIPS_ANGLE_D90;
        break;
    case 180:
        angle = VIPS_ANGLE_D180;
        break;
    case 270:
        angle = VIPS_ANGLE_D270;
        break;
    default:
        return FAIL;
    }

    if (vips_rot(input, output, angle, NULL)) {
        v_vips_err("rotating %d", deg);
        return FAIL;
    }

    return OK;
}

RESULT autorot_image(VipsImage *input, VipsImage **output) {
    v_log(INFO, "autorotate");
    if (vips_autorot(input, output, NULL)) {
        v_vips_err("autorotating");
        return FAIL;
    }
    return OK;
}

RESULT embed_image(VipsImage *input, VipsImage **output, int x, int y, int width, int height, VipsExtend extend) {
    // https://libvips.github.io/libvips/API/current/libvips-conversion.html#vips-embed
    // https://libvips.github.io/libvips/API/current/libvips-conversion.html#VipsExtend

    int curWidth = vips_image_get_width(input);
    int curHeight = vips_image_get_height(input);

    v_log(INFO, "embedding image with size %dx%d at %d,%d in image of size %dx%d", curWidth, curHeight, x, y, width, height);

    if (x < 0 || y < 0) {
        v_log(ERROR, "embed location %d,%d is less than zero point", x, y);
        return FAIL;
    }

    if (width < 1 || height < 1) {
        v_log(ERROR, "embed dimensions must both exceed 1 %d %d", width, height);
        return FAIL;
    }

    if (vips_embed(input, output, x, y, width, height, "extend", extend, NULL)) {
        v_vips_err("embedding image with size %dx%d at %d,%d in image of size "
                   "%dx%d failed",
                   curWidth, curHeight, x, y, width, height);
        return FAIL;
    }

    return OK;
}

RESULT embed_image_white(VipsImage *in, VipsImage **out, int x, int y, int width, int height) {
    return embed_image(in, out, x, y, width, height, VIPS_EXTEND_WHITE);
}

RESULT embed_image_black(VipsImage *in, VipsImage **out, int x, int y, int width, int height) {
    return embed_image(in, out, x, y, width, height, VIPS_EXTEND_BLACK);
}

RESULT extract_image(VipsImage *input, VipsImage **output, int left, int top, int width, int height) {
    v_log(INFO, "extracting %d %d %d %d\n", left, top, width, height);

    if (left < 0 || top < 0) {
        v_log(ERROR, "extract origin %d,%d is less than zero point", left, top);
        return FAIL;
    }

    if (width < 1 || height < 1) {
        v_log(ERROR, "extract dimensions must both exceed 1 %d %d", width, height);
        return FAIL;
    }

    int img_height = vips_image_get_height(input);
    int img_width = vips_image_get_width(input);

    // TODO: Double check this is correct
    if (img_height < height + top || img_width < width + left) {
        v_log(ERROR, "extract size (%d,%d) is outside image bounds (%d,%d)", width, height, img_width + left, img_height + top);
        return FAIL;
    }

    if (vips_extract_area(input, output, left, top, width, height, NULL)) {
        v_vips_err("extracting %d,%d,%d,%d failed", width, height, img_width, img_height);
        return FAIL;
    }
    return OK;
}

RESULT gaussblur_image(VipsImage *input, VipsImage **output, double sigma) {
    v_log(INFO, "gaussian blur sigma %f", sigma);
    if (vips_gaussblur(input, output, sigma, NULL)) {
        v_vips_err("gaussian blur sigma %f", sigma);
        return FAIL;
    }
    return OK;
}

RESULT resize_image(VipsImage *input, VipsImage **output, int height, int width, unsigned opts) {
    char *resize_op = NULL;

    if (height < 1) {
        v_log(ERROR, "width must be > 1, not %d", height);
        return FAIL;
    }

    if (height < 1) {
        v_log(ERROR, "height must be > 1, not %d", width);
        return FAIL;
    }

    int curHeight = vips_image_get_height(input);
    int curWidth = vips_image_get_width(input);

    double heightRatio = (double)height / (double)curHeight;
    double widthRatio = (double)width / (double)curWidth;

    double hScale = 0;
    double vScale = 0;

    if (opts & STRETCH) {
        resize_op = "stretch";
        hScale = widthRatio;
        vScale = heightRatio;
    } else if (opts & EXPAND) {
        resize_op = "expand";
        if (heightRatio > widthRatio) {
            hScale = heightRatio;
            vScale = heightRatio;
        } else {
            hScale = widthRatio;
            vScale = widthRatio;
        }
    } else if (opts & RESIZE) {
        resize_op = "resize";
        if (heightRatio < widthRatio) {
            hScale = heightRatio;
            vScale = heightRatio;
        } else {
            hScale = widthRatio;
            vScale = widthRatio;
        }
    } else {
        v_log(ERROR, "unknown resize mode: 0x%x", opts);
        return FAIL;
    }

    if (vips_resize(input, output, hScale, "vscale", vScale, NULL)) {
        v_vips_err("%s (%d,%d)", resize_op, curWidth, curHeight);
        return FAIL;
    }

    int actWidth = vips_image_get_width(*output);
    int actHeight = vips_image_get_height(*output);

    v_log(INFO, "%s (%d,%d) -> (%d,%d)", resize_op, curWidth, curHeight, actWidth, actHeight);
    return OK;
}

RESULT export_image(VipsImage *input, Buffer *output, format_t format, int quality) {
    v_log(INFO, "exporting to %s with quality %d", get_format_name(format), quality);

    int vips_result = 1;
    switch (format) {
    case JPEG:
        vips_result = vips_jpegsave_buffer(input, &(output->data), &(output->len), "Q", quality, NULL);
        break;
    case PNG:
        vips_result = vips_pngsave_buffer(input, &(output->data), &(output->len), "Q", quality, "compression", 0, NULL);
        break;
    case WEBP:
        vips_result = vips_webpsave_buffer(input, &(output->data), &(output->len), "Q", quality, NULL);
        break;
    case TIFF:
        vips_result = vips_tiffsave_buffer(input, &(output->data), &(output->len), "Q", quality, NULL);
        break;
    case GIF:
        vips_result = vips_magicksave_buffer(input, &(output->data), &(output->len), "quality", quality, "format", "gif", NULL);
        break;
    case BMP:
        vips_result = vips_magicksave_buffer(input, &(output->data), &(output->len), "quality", quality, "format", "bmp", NULL);
        break;
    default:
        v_log(ERROR, "invalid format selected");
        return FAIL;
    }

    int width = vips_image_get_width(input);
    int height = vips_image_get_height(input);
    // int height = 0;
    // int width = 0;

    if (vips_result) {
        v_vips_err("error exporting to %d, %d %s", width, height, get_format_name(format));
        return FAIL;
    }
    return OK;
}
