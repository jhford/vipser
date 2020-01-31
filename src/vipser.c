#include <unistd.h>
#include <vips/vips.h>
#include "vips_glue.h"
#include "types.h"
#include "io.h"
#include "sniffing.h"

int main(int argc, char **argv) {

    // Initialise sniffing
    magic_init();

    // VipsImages store the in-process images
    VipsImage *in = NULL;
    VipsImage *out = NULL;

    // Output format and quality
    format_t out_format = WEBP;
    int out_quality = DEFAULT_QUALITY;

    // Read in all of STDIN
    char *input_buf = NULL;
    size_t input_size = 0;
    read_all(STDIN_FILENO, &input_size, &input_buf);

    // Determine which file type the input image is, set that as the default
    if (OK != determine_buffer_type(input_size, input_buf, &out_format)) {
        fprintf(stderr, "error determining buffer type\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the Vips library
    if(VIPS_INIT(argv[0])){
        vips_error_exit("initialising vips");
    }

    // Load the input into a VipsImage
    in = vips_image_new_from_buffer(input_buf, input_size, "", NULL);
    if (in == NULL) {
        vips_error_exit("reading image to buffer");
    }

    // We no longer need the raw input, so we can free it
    free(input_buf);

    // Each command line argument is an instruction for this program to perform
    // on an image.  Each operation occurs completely independently of all others
    // and so results of previous operations cannot be referenced in later ones
    for (int i = 1; i < argc; i++) {
        fprintf(stderr, "processing command %s...\n", argv[i]);
        int edited = 0;

        run_command(in, &out, &out_format, &out_quality, &edited, argv[i]);

        // If the image was mutated, swap the output for the input and release
        // the old input's memory
        if (edited) {
            g_object_unref(in);
            in = out;
        }

        fprintf(stderr, "done!\n");
    }

    // Export the image as a buffer, which is allocated internally with glib by
    // vips
    void* out_buf = NULL;
    size_t out_size = 0;
    if (OK != export_image(in, &out_buf, &out_size, out_format, out_quality)) {
        vips_error_exit("writing output");
    }

    // Write the resulting image to STDOUT
    if (-1 == write(STDOUT_FILENO, out_buf, out_size)) {
        perror("writing output");
        exit(EXIT_FAILURE);
    }

    // Free the copy of the image's output
    g_free(out_buf);
}