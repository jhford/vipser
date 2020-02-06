#include <magic.h>
#include <stdio.h>
#include <string.h>

#include "io.h"
#include "types.h"

// The libmagic cookie to be used by all calls.  This is
// set by magic_init()
magic_t cookie;

char *get_format_name(format_t fmt) {
    switch (fmt) {
    case JPEG:
        return JPEG_NAME;
    case PNG:
        return PNG_NAME;
    case WEBP:
        return WEBP_NAME;
    case TIFF:
        return TIFF_NAME;
    case GIF:
        return GIF_NAME;
    case BMP:
        return BMP_NAME;
    default:
        return UNK_NAME;
    }
}

format_t format_from_name(const char *name) {
    if (0 == strncmp(JPEG_NAME, name, strlen(JPEG_NAME)) || 0 == strncmp(JPEG_NAME2, name, strlen(JPEG_NAME2))) {
        return JPEG;
    } else if (0 == strncmp(PNG_NAME, name, strlen(PNG_NAME))) {
        return PNG;
    } else if (0 == strncmp(WEBP_NAME, name, strlen(WEBP_NAME))) {
        return WEBP;
    } else if (0 == strncmp(TIFF_NAME, name, strlen(TIFF_NAME))) {
        return TIFF;
    } else if (0 == strncmp(GIF_NAME, name, strlen(GIF_NAME))) {
        return GIF;
    } else if (0 == strncmp(BMP_NAME, name, strlen(BMP_NAME))) {
        return BMP;
    }

    return UNK;
}

format_t format_from_mime(const char *mime) {
    if (0 == strncmp(JPEG_MIME, mime, strlen(JPEG_MIME))) {
        return JPEG;
    } else if (0 == strncmp(PNG_MIME, mime, strlen(PNG_MIME))) {
        return PNG;
    } else if (0 == strncmp(WEBP_MIME, mime, strlen(WEBP_MIME))) {
        return WEBP;
    } else if (0 == strncmp(TIFF_MIME, mime, strlen(TIFF_MIME))) {
        return TIFF;
    } else if (0 == strncmp(GIF_MIME, mime, strlen(GIF_MIME))) {
        return GIF;
    } else if (0 == strncmp(BMP_MIME1, mime, strlen(BMP_MIME1)) || 0 == strncmp(BMP_MIME2, mime, strlen(BMP_MIME2))) {
        return BMP;
    }

    return UNK;
}

// Initialise libmagic and set the global magic cookie.  This function
// should only be called once.
RESULT magic_init() {
    // TODO we should embed the image magic database into this program so that
    // we don't depend on anything on the local filesystem
    cookie = magic_open(MAGIC_MIME_TYPE);

    if (0 != magic_load(cookie, NULL)) {
        v_log(ERROR, "magic: %s", magic_error(cookie));
        return FAIL;
    }

    return OK;
}

RESULT determine_descriptor_type(int fd, char **name) {
    format_t fmt;
    const char *type = magic_descriptor(cookie, fd);

    if (type == NULL) {
        v_log(ERROR, "magic: %s", magic_error(cookie));
        return FAIL;
    }

    fmt = format_from_mime(type);
    *name = get_format_name(fmt);

    return OK;
}

RESULT determine_buffer_type(Buffer *input, format_t *fmt) {
    u_int8_t *buf = (u_int8_t *)input->data;

    *fmt = UNK;

    // Shortcut the buffer type if we're reasonably certain about the image type
    if (input->len > 12) {
        if (buf[0] == 0xFF && buf[1] == 0xD8 && buf[2] == 0xFF) {
            *fmt = JPEG;
        }
        if (buf[0] == 0x47 && buf[1] == 0x49 && buf[2] == 0x46) {
            *fmt = GIF;
        }
        if (buf[0] == 0x42 && buf[1] == 0x4D) {
            *fmt = BMP;
        }
        if (buf[0] == 0x89 && buf[1] == 0x50 && buf[2] == 0x4E && buf[3] == 0x47) {
            *fmt = PNG;
        }
        if ((buf[0] == 0x49 && buf[1] == 0x49 && buf[2] == 0x2A && buf[3] == 0x0) ||
            (buf[0] == 0x4D && buf[1] == 0x4D && buf[2] == 0x0 && buf[3] == 0x2A)) {
            *fmt = TIFF;
        }
        if (buf[8] == 0x57 && buf[9] == 0x45 && buf[10] == 0x42 && buf[11] == 0x50) {
            *fmt = WEBP;
        }
    }

    if (*fmt != UNK) {
        return OK;
    }

    v_log(INFO, "using libmagic to sniff");
    const char *type = magic_buffer(cookie, input->data, input->len);

    if (type == NULL) {
        fprintf(stdout, "magic: %s", magic_error(cookie));
        return FAIL;
    }

    *fmt = format_from_mime(type);

    return OK;
}
