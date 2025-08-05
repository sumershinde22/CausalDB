#ifndef REPL_H
#define REPL_H

#include <stddef.h>
#include <unistd.h>

typedef struct {
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

InputBuffer* new_input_buffer();
void read_input(InputBuffer* ib);
void close_input_buffer(InputBuffer* ib);

#endif