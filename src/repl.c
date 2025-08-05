#include "repl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InputBuffer* new_input_buffer() {
    InputBuffer* ib = malloc(sizeof(InputBuffer));
    ib->buffer = NULL;
    ib->buffer_length = 0;
    ib->input_length = 0;
    return ib;
}

void read_input(InputBuffer* ib) {
    ssize_t bytes_read = getline(&ib->buffer, &ib->buffer_length, stdin);
    if (bytes_read <= 0) {
        printf("Error reading input\n");
        exit(1);
    }

    // Strip newline
    if (ib->buffer[bytes_read - 1] == '\n') {
        ib->buffer[bytes_read - 1] = '\0';
        ib->input_length = bytes_read - 1;
    } else {
        ib->input_length = bytes_read;
    }
}

void close_input_buffer(InputBuffer* ib) {
    free(ib->buffer);
    free(ib);
}