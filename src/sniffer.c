#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "sniffing.h"
#include "io.h"


int main(int argc, char **argv) {
    char* type = NULL;

    if (OK != magic_init()) {
        perror("sniffer");
        exit(EXIT_FAILURE);
    }

    if (OK != determine_descriptor_type(STDIN_FILENO, &type)) {
        perror("sniffer");
        exit(EXIT_FAILURE);
    }

    printf("%s\n", type);
}