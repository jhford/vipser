#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "types.h"
#include "io.h"

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
