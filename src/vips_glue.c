#include <vips/vips.h>
#include <stdio.h>
#include <sys/errno.h>

#include "vips_glue.h"
#include "sniffing.h"

RESULT parse_int(char *val, int *out) {
    errno = 0;

    long long_v = strtol(val, NULL, 0);

    if (errno != 0) {
        perror(val);
        return FAIL;
    }

    if (long_v > INT_MAX) {
        fprintf(stderr, "parse_int: %s is larger than allowed max of %d\n", val, INT_MAX);
        return FAIL;
    }

    *out = (int) long_v;

    return OK;
}

RESULT parse_dbl(char *val, double *out) {
    errno = 0;

    *out = strtod(val, NULL);

    if (errno != 0) {
        perror(val);
        *out = 0;
        return FAIL;
    }

    return OK;
}

RESULT rotate_image(VipsImage *in, VipsImage **out, int deg) {
    VipsAngle angle;
    switch (deg) {
        case 0:
        case 360:
            fprintf(stderr, "skipping ineffective rotation\n");
            return OK;
        case 90:
            angle = VIPS_ANGLE_D90;
            fprintf(stderr, "rotating 90deg\n");
            break;
        case 180:
            angle = VIPS_ANGLE_D180;
            fprintf(stderr, "rotating 180deg\n");
            break;
        case 270:
            angle = VIPS_ANGLE_D270;
            fprintf(stderr, "rotating 270deg\n");
            break;
        default:
            return FAIL;
    }
    if (0 == vips_rot(in, out, angle, NULL)) {
        return OK;
    }
    return FAIL;
}

RESULT autorot_image(VipsImage *in, VipsImage **out) {
    fprintf(stderr, "autorotate\n");
    if (0 == vips_autorot(in, out, NULL)) {
        return OK;
    }
    return FAIL;
}

RESULT extract_image(VipsImage *in, VipsImage **out, int left, int top, int width, int height) {
    fprintf(stderr, "extract %d %d %d %d\n", left, top, width, height);
    if (0 == vips_extract_area(in, out, left, top, width, height, NULL)) {
        return OK;
    }
    return FAIL;
}

RESULT gaussblur_image(VipsImage *in, VipsImage **out, double sigma) {
    fprintf(stderr, "gaussian blur %f\n", sigma);
    if (0 == vips_gaussblur(in, out, sigma, NULL)){
        return OK;
    }
    return FAIL;
}


RESULT resize_image(VipsImage *in, VipsImage **out, int newHeight, int newWidth, unsigned opts) {
    int curHeight = vips_image_get_height(in);
    int curWidth = vips_image_get_width(in);

    double heightRatio = (double)newHeight / (double)curHeight;
    double widthRatio = (double)newWidth / (double)curWidth;

    double hScale = 0;
    double vScale = 0;

    if (opts & STRETCH) {
        fprintf(stderr, "stretch %f %f\n", widthRatio, heightRatio);
        hScale = widthRatio;
        vScale = heightRatio;
    } else if (opts & EXPAND) {
        if (heightRatio > widthRatio) {
            fprintf(stderr, "expanding %f\n", heightRatio);
            hScale = heightRatio;
            vScale = heightRatio;
        } else {
            fprintf(stderr, "expanding %f\n", widthRatio);
            hScale = widthRatio;
            vScale = widthRatio;
        }
    } else {
        if (heightRatio < widthRatio) {
            fprintf(stderr, "resizing %f\n", heightRatio);
            hScale = heightRatio;
            vScale = heightRatio;
        } else {
            fprintf(stderr, "resizing %f\n", widthRatio);
            hScale = widthRatio;
            vScale = widthRatio;
        }
    }

    if (0 == vips_resize(in, out, hScale, "vscale", vScale, NULL)) {
        return OK;
    }
    // TODO: Print Vips Error
    return FAIL;
}

RESULT get_command_args(char* cmd, int *argc, char **argv) {
    char *sep = ",";

    *argc=0;

    for (char *token = strtok(cmd, sep); token; *argc = *argc + 1, token = strtok(NULL, sep)) {
        argv[*argc] = token;
    }

    return OK;
}


// TODO:
// - stretch & expand
RESULT run_command(VipsImage *in, VipsImage **out, format_t* format, int* quality, int* edited, char* cmd) {
    size_t arg_len = strnlen(cmd, 100);

    int commas = 0;
    int argc = 0;
    for (int i = 0; i < strlen(cmd); i++) {
        if (cmd[i] == ',') {
            commas++;
        }
    }

    char *argv[commas + 1];

    if (OK != get_command_args(cmd, &argc, argv)) {
        return FAIL;
    }

    /// RESIZE
    if (argc == 3 && 0 == strcmp("RESIZE", argv[0])) {
        int width;
        int height;
        if (OK != parse_int(argv[1], &width)) {
            return FAIL;
        }
        if (OK != parse_int(argv[2], &height)) {
            return FAIL;
        }
        if (OK != resize_image(in, out, height, width, 0)) {
            return FAIL;
        }
        *edited = 1;

    /// EXTRACT
    } else if (argc == 5 && 0 == strcmp("EXTRACT", argv[0])) {
        int left;
        int top;
        int width;
        int height;
        if (OK != parse_int(argv[1], &left)) {
            return FAIL;
        }
        if (OK != parse_int(argv[2], &top)) {
            return FAIL;
        }
        if (OK != parse_int(argv[3], &width)) {
            return FAIL;
        }
        if (OK != parse_int(argv[4], &height)) {
            return FAIL;
        }
        if (OK != extract_image(in, out, left, top, width, height)) {
            return FAIL;
        }
        *edited = 1;

    /// BLUR
    } else if (argc == 2 && 0 == strcmp("BLUR", argv[0])) {
        *edited = 1;
        double sigma;
        if (OK != parse_dbl(argv[1], &sigma)) {
            return FAIL;
        }
        if (OK != gaussblur_image(in, out, sigma)){
            return FAIL;
        }
        *edited = 1;

    /// ROTATE
    } else if (argc == 2 && 0 == strcmp("ROTATE", argv[0])) {
        int rotation;
        if (OK != parse_int(argv[1], &rotation)) {
            return FAIL;
        }
        if (OK != rotate_image(in, out, rotation)){
            return FAIL;
        }
        *edited = 1;

    /// AUTOROT
    } else if (argc == 1 && 0 == strcmp("AUTOROT", argv[0])) {
        if (OK != autorot_image(in, out)){
            return FAIL;
        }
        *edited = 1;

    /// QUALITY
    } else if (argc == 2 && 0 == strcmp("QUALITY", argv[0])) {
        *edited = 0;
        *quality = DEFAULT_QUALITY;
        if (OK != parse_int(argv[1], quality)) {
            return FAIL;
        }

    /// EXPORT
    } else if (argc == 2 && 0 == strcmp("EXPORT", argv[0])) {
        *format = format_from_name(argv[1]);
    }

    return OK;
}

RESULT export_image(VipsImage* in, void** out, size_t *n, format_t format, int quality) {
    switch (format) {
        case JPEG:
            vips_jpegsave_buffer(in, out, n, "Q", quality, NULL);
            break;
        case PNG:
            vips_pngsave_buffer(in, out, n, "Q", quality, "compression", 0, NULL);
            break;
        case WEBP:
            vips_webpsave_buffer(in, out, n, "Q", quality, NULL);
            break;
        case TIFF:
            vips_tiffsave_buffer(in, out, n, "Q", quality, NULL);
            break;
        case GIF:
            vips_magicksave_buffer(in, out, n, "quality", quality, "format", "gif", NULL);
            break;
        case BMP:
            vips_magicksave_buffer(in, out, n, "quality", quality, "format", "bmp", NULL);
            break;
        default:
            fprintf(stderr, "invalid format selected\n");
            return FAIL;
    }
    return OK;
}
