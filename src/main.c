#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "event.h"
#include "db.h"
#include "statement.h"
#include "repl.h"

void print_event(Event* e) {
    printf("%u: %s\n", e->id, e->data);
    printf(" ⬑ Parents:");
    for (int i = 0; i < e->parent_count; i++) {
        printf(" %u", e->parents[i]);
    }
    printf("\n");
}

int main() {
    open_db("causal.cdb");

    Table* table = load_table("causal.cdb");

    InputBuffer* input_buffer = new_input_buffer();

    while (1) {
        printf("db > ");
        read_input(input_buffer);

        if (strncmp(input_buffer->buffer, ".exit", 5) == 0) {
            break;
        } else if (strncmp(input_buffer->buffer, ".list", 5) == 0) {
            for (size_t i = 0; i < table->num_events; i++) {
                print_event(&table->events[i]);
            }
            continue;
        }

        Statement stmt;
        StatementType type = parse_statement(input_buffer->buffer, &stmt);

        if (type == STATEMENT_UNKNOWN) {
            printf("Unrecognized command: %s\n", input_buffer->buffer);
            continue;
        }

        stmt.type = type;
        execute_statement(&stmt, table);
    }

    close_db();
    free(table);
    close_input_buffer(input_buffer);
    return 0;
}
