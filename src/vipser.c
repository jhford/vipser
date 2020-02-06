#include <fcntl.h>
#include <sys/errno.h>
#include <time.h>
#include <unistd.h>
#include <vips/vips.h>

#include "commands.h"
#include "io.h"
#include "sniffing.h"
#include "types.h"
#include "vips_glue.h"

#define OVERRIDE_INPUT "input"
#define OVERRIDE_OUTPUT "output"

int get_input_fd() {
#ifdef OVERRIDE_INPUT
    int fd;
    errno = 0;
    fd = open(OVERRIDE_INPUT, O_RDONLY);
    if (fd < 0) {
        v_log_errno(errno, "opening input file %s", OVERRIDE_INPUT);
        exit(EXIT_FAILURE);
    }
    return fd;
#else
    return STDIN_FILENO;
#endif
}

int get_output_fd() {
#ifdef OVERRIDE_OUTPUT
    int fd;
    errno = 0;
    fd = open(OVERRIDE_OUTPUT, O_WRONLY | O_SYNC | O_TRUNC | O_CREAT, 0666);
    if (fd < 0) {
        v_log_errno(errno, "opening output file %s", OVERRIDE_OUTPUT);
        exit(EXIT_FAILURE);
    }
    return fd;
#else
    return STDIN_FILENO;
#endif
}

int main(int argc, char **argv) {

    // Output format and quality
    format_t format = JPEG;
    int quality = DEFAULT_QUALITY;

    // The input and output buffers
    Buffer input;
    input.len = 0;
    input.data = NULL;

    Buffer output;
    output.len = 0;
    output.data = NULL;

    // Miscellaneous variables
    size_t bytes_written = 0;

    // Initialise logging
    v_log_init();
    v_log(DEBUG, "logging initialised");

    // Input file descriptors
    int input_fd = get_input_fd();
    int output_fd = get_output_fd();

    // Initialise sniffing
    if (OK != magic_init()) {
        v_log(ERROR, "error initialising libmagic");
        exit(EXIT_FAILURE);
    }
    v_log(DEBUG, "magic initialised");

    // Initialise the Vips
    if (OK != init_vips(argv[0])) {
        v_log(ERROR, "error initialising vips");
        exit(EXIT_FAILURE);
    }
    v_log(DEBUG, "vips initialised");

    // Read all the input from the input file
    if (OK != read_input(input_fd, &input, &format)) {
        v_log(ERROR, "error reading input");
        exit(EXIT_FAILURE);
    }
    v_log(INFO, "read %d bytes", input.len);

    // Close the input file as soon as possible
    errno = 0;
    if (close(input_fd)) {
        v_log_errno(errno, "closing input");
        exit(EXIT_FAILURE);
    }

    // Enable concurrency -- TODO benchmark whether this is worthwhile
    vips_concurrency_set(8);

    // Run all the commands.
    if (OK != run_commands(argc, argv, input, &output, &format, &quality)) {
        v_log(ERROR, "error running commands");
        exit(EXIT_FAILURE);
    }

    // Write the output file
    errno = 0;
    bytes_written = write(output_fd, output.data, output.len);
    if (0 > bytes_written) {
        v_log_errno(errno, "writing output");
        exit(EXIT_FAILURE);
    }
    v_log(INFO, "wrote %d bytes of output", bytes_written);

    // Close the input and output
    errno = 0;
    if (close(output_fd)) {
        v_log_errno(errno, "closing output file");
        exit(EXIT_FAILURE);
    }

    // Cleanup memory
    free(input.data);
    g_free(output.data);

    // Shutdown Vips
    vips_thread_shutdown();
    vips_shutdown();
}
