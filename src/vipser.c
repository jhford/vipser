#include <unistd.h>
#include <vips/vips.h>
#include <time.h>
#include <sys/errno.h>
#include <fcntl.h>

#include "vips_glue.h"
#include "types.h"
#include "io.h"
#include "sniffing.h"
#include "commands.h"

#define OVERRIDE_INPUT "input"
#define OVERRIDE_OUTPUT "output"

int main(int argc, char **argv) {
    v_log_init();
    v_log(INFO, "startup");

    // Initialise sniffing
    magic_init();
    v_log(INFO, "magic init");

    // Initialize the Vips library
    if(VIPS_INIT(argv[0])){
        v_vips_err("initialising vips");
        exit(EXIT_FAILURE);
    }
    v_log(INFO, "vips init");

    // VipsImages store the image-process images
    VipsImage *image = NULL;

    // Output format and quality
    format_t out_format = JPEG;
    int out_quality = DEFAULT_QUALITY;

    // Read image all of STDIN.  This memory is used internally by Vips
    // to back the VipsImages
    char *input_buf = NULL;
    size_t input_size = 0;
#ifdef OVERRIDE_INPUT
    int fd = open(OVERRIDE_INPUT, O_RDONLY);
    if (OK != read_all(fd, &input_size, &input_buf)) {
#else
    if (OK != read_all(STDIN_FILENO, &input_size, &input_buf)) {
#endif
        v_log(ERROR, "error reading input");
        exit(EXIT_FAILURE);
    }
    v_log(INFO, "read %d bytes of input", input_size);
#ifdef OVERRIDE_INPUT
    close(fd);
#endif

    // Determine which file type the input image is, set that as the default
    if (OK != determine_buffer_type(input_size, input_buf, &out_format)) {
        v_log(ERROR, "error determining buffer type");
        exit(EXIT_FAILURE);
    }
    v_log(INFO, "input type determined: %s", get_format_name(out_format));

    vips_concurrency_set(8);

    // Load the input into a VipsImage
    image = vips_image_new_from_buffer(input_buf, input_size, "", NULL);
    if (image == NULL) {
        v_vips_err("error reading image from buffer");
        exit(EXIT_FAILURE);
    }
    v_log(INFO, "create input image");

    // We no longer need the raw input, so we can free it

    // Each command line argument is an instruction for this program to perform
    // on an image.  Each operation occurs completely independently of all others
    // and so results of previous operations cannot be referenced image later ones
    for (int i = 1; i < argc; i++) {
        v_log(INFO, "initiating command %s", argv[i]);

        if (OK != run_command(&image, &out_format, &out_quality, argv[i])) {
            v_log(ERROR, "running command: %s", argv[i]);
            exit(EXIT_FAILURE);
        }

        v_log(INFO, "command complete");
    }

    // Export the image as a buffer, which is allocated internally with glib by
    // vips
    void* out_buf = NULL;
    size_t out_size = 0;
    if (OK != export_image(image, &out_buf, &out_size, out_format, out_quality)) {
        v_log(ERROR, "exporting image");
        exit(EXIT_FAILURE);
    }
    v_log(INFO, "exported %d bytes of %s", out_size, get_format_name(out_format));

    // We're finally done with the image
    g_object_unref(image);

    // Export Image
    size_t bytes_written = 0;
    // Write the resulting image to STDOUT
    errno = 0;
#ifdef OVERRIDE_OUTPUT
    int fd2 = open(OVERRIDE_OUTPUT, O_WRONLY|O_SYNC|O_TRUNC|O_CREAT, 0666);
    if (0 > (bytes_written = write(fd2, out_buf, out_size))) {
#else
    if (0 > (bytes_written = write(STDOUT_FILENO, out_buf, out_size))) {
#endif
        v_log_errno(ERROR, errno);
        exit(EXIT_FAILURE);
    }
    v_log(INFO, "wrote %d bytes of output", bytes_written);

#ifdef OVERRIDE_OUTPUT
    close(fd2);
#endif

    // Cleanup memory
    free(input_buf);
    input_buf = NULL;

    g_free(out_buf);
    out_buf = NULL;

    // Shutdown Vips
    vips_shutdown();
}