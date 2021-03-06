#ifndef VIPSER_TYPES_H
#define VIPSER_TYPES_H

typedef int RESULT;
#define OK 0
#define FAIL 1

// boolean names are nicer
typedef int bool_t;
#define true 1
#define false 0

// Resizing strategies
#define RESIZE 1u
#define STRETCH 2u
#define EXPAND 4u

// Formats
typedef int format_t;
#define UNK 0
#define JPEG 0x1
#define PNG 0x2
#define WEBP 0x3
#define TIFF 0x4
#define GIF 0x5
#define BMP 0x6

// Names used in IPC
#define JPEG_NAME "jpg"
#define JPEG_NAME2 "jpeg"
#define PNG_NAME "png"
#define WEBP_NAME "webp"
#define TIFF_NAME "tiff"
#define GIF_NAME "gif"
#define BMP_NAME "bmp"
#define UNK_NAME "<unknown>"

// MIME Types as returned by libmagic
#define JPEG_MIME "image/jpeg"
#define PNG_MIME "image/png"
#define WEBP_MIME "image/webp"
#define TIFF_MIME "image/tiff"
#define GIF_MIME "image/gif"
#define BMP_MIME1 "image/x-ms-bmp"
#define BMP_MIME2 "image/bmp"

// A buffer stores data and a size of the data
typedef struct Buffer {
    size_t len;
    void *data;
} Buffer;

// reset a buffer to its zero state
static inline void BufferClear(Buffer *buf) {
    buf->data = NULL;
    buf->len = 0;
}

#endif // VIPSER_TYPES_H
