#include "io.h"
#include "sniffing.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    char *type = NULL;

    // Initialise libmagic
    if (OK != magic_init()) {
        perror("sniffer");
        exit(EXIT_FAILURE);
    }

    // Determine the file type for the file descriptor
    if (OK != determine_descriptor_type(STDIN_FILENO, &type)) {
        perror("sniffer");
        exit(EXIT_FAILURE);
    }

    printf("%s\n", type);
    exit(EXIT_SUCCESS);
}