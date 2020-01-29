#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <vips/vips.h>

#define BUFFER_SIZE 4096

void print_array(char* a, size_t n) {
    for (int i = 0 ; i < n ; i++) {
        printf("0x%x", a[i]);
        if (i % 40 == 0) {
            printf("\n");
        } else {
            printf(" ");
        }
    }
}

// Need:
//  - sniff, return height, width and format
//  - resize
//  - stretch
//  - output

int main(int argc, char **argv) {
    char *buf = NULL;
    char *input = NULL;
    size_t inputSize = 0;
    VipsImage *in;

    if(VIPS_INIT(argv[0])) {
        vips_error_exit(NULL);
    }

    printf("vips initialized\n");

    // Allocate read buffer
    buf = malloc(BUFFER_SIZE);
    if (NULL == buf) {
        perror(NULL);
        exit(EXIT_FAILURE);
    }
    printf("buffer allocated\n");

    // Read in the full contents of STDIN
    for (size_t i = 0, nBytes = 1; ; i++) {
        nBytes = read(STDIN_FILENO, buf, BUFFER_SIZE);
        if (nBytes == -1) {
            perror(NULL);
            exit(EXIT_FAILURE);
        }

        inputSize += nBytes;

        printf("read %zu bytes, %zu total\n", nBytes, inputSize);

        if (nBytes == 0) {
            break;
        }

        input = realloc(input, inputSize);

        // Yikes this is uuuugly pointer arithmetic but...
        memcpy(input + (i * BUFFER_SIZE), buf, nBytes);
    }

    // Read in the image
    in = vips_image_new_from_buffer(input, inputSize, "", NULL);
    if (NULL == in) {
        vips_error_exit(NULL);
    }
    printf("created new VipsImage\n");

    printf("height %d width %d\n", vips_image_get_height(in), vips_image_get_height(in));



}