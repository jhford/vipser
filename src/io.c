#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include "types.h"
#include "io.h"
#include <time.h>
#include <sys/errno.h>
#include <vips/vips.h>

// Read an entire file descriptor into a buffer pointed to by `out` of size `size`
RESULT read_all(int fd, size_t *size, char **out) {
    char *buf = NULL;
    char *input = NULL;
    size_t input_size = 0;
    int n_bytes = 0;

    // Allocate reading buffer
    buf = malloc(BUFFER_SIZE);

    // Read in the bytes
    for (size_t i = 0; ; i++){
        n_bytes = read(fd, buf, BUFFER_SIZE);
        if (n_bytes == -1) {
            perror("read_all");
            return FAIL;
        }

        input_size += n_bytes;

        if (n_bytes == 0) {
            break;
        }

        input = realloc(input, input_size);

        // Yikes this is uuuugly pointer arithmetic but...
        // If you have a better way to do this, please let me know
        memcpy(input + (i * BUFFER_SIZE), buf, n_bytes);
    }

    *size = input_size;
    *out = input;

    free(buf);
    return OK;
}

struct timespec prog_start;

void v_log_init() {
    if (-1 == clock_gettime(CLOCK_MONOTONIC, &prog_start)) {
        perror("getting program start time");
        exit(-1);
    }
}

// Ideally this would be variadic and take a decorating string
void v_log_errno(level lvl, int err) {
    errno = 0;
    char *err_s = strerror(err);

    if (0 != errno) {
        perror("error logging errno");
        exit(EXIT_FAILURE);
    }
    v_log(lvl, err_s);
}

void v_vips_err(const char *fmt, ...) {

    // Build the specified message
    va_list args;
    va_start(args, fmt);
    int n = vsprintf(NULL, fmt, args);
    char* errmsg = malloc(sizeof(char) * n);
    if (-1 == vsprintf(errmsg, fmt, args)) {
        perror("v_vips_err");
        exit(EXIT_FAILURE);
    }
    va_end(args);

    // Now append the Vips error
    const char *vips_err_buf = vips_error_buffer();
    n = sprintf(NULL, "%s: %s", errmsg, vips_err_buf);
    char* errmsg2 = malloc(sizeof(char) * n);
    n = sprintf(errmsg2, "%s: %s", errmsg, vips_error_buffer());

    free(errmsg);

    v_log(ERROR, errmsg2);

    free(errmsg2);
}

void v_log(level lvl, const char *fmt, ...) {
    char* lvl_string = NULL;

    switch(lvl) {
        case DEBUG:
            lvl_string = "DEBUG";
            break;
        case INFO:
            lvl_string = "INFO ";
            break;
        case WARN:
            lvl_string = "WARN ";
            break;
        case ERROR:
            lvl_string = "ERROR";
            break;
    }

    struct timespec rcvd;

    if (-1 == clock_gettime(CLOCK_MONOTONIC, &rcvd)) {
        perror("getting v_log time");
    }

    struct timespec diff;
    if ((rcvd.tv_nsec - prog_start.tv_nsec) < 0) {
        diff.tv_sec = rcvd.tv_sec - prog_start.tv_sec - 1;
        diff.tv_nsec = rcvd.tv_nsec - prog_start.tv_nsec + 1000000000;
    } else {
        diff.tv_sec = rcvd.tv_sec - prog_start.tv_sec;
        diff.tv_nsec = rcvd.tv_nsec - prog_start.tv_nsec;
    }

    // TODO: This is totally invalid in the case where the two tv_sec members are differnt
    // and likely just generally broken

    fprintf(stderr,"%s - %3lld.%.9ld: ", lvl_string, (long long)diff.tv_sec, diff.tv_nsec);

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");
    fsync(STDERR_FILENO);
}