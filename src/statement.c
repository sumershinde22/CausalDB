#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "db.h"
#include "statement.h"

StatementType parse_statement(const char* input, Statement* statement) {
    if (strncmp(input, "insert", 6) == 0) {
        statement->type = STATEMENT_INSERT;

        Event* e = &statement->event;
        e->parent_count = 0;

        char buffer[256];
        strncpy(buffer, input + 7, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';

        char* token = strtok(buffer, " ");
        if (!token) return STATEMENT_UNKNOWN;
        e->id = atoi(token);

        token = strtok(NULL, "\"");
        if (!token) return STATEMENT_UNKNOWN;
        strncpy(e->data, token, MAX_DATA_LENGTH);

        token = strtok(NULL, " ");
        while (token && e->parent_count < MAX_PARENTS) {
            e->parents[e->parent_count++] = atoi(token);
            token = strtok(NULL, " ");
        }

        return STATEMENT_INSERT;
    } else if (strncmp(input, "get", 3) == 0) {
        statement->type = STATEMENT_GET;
        statement->query_id = atoi(input + 4);
        return STATEMENT_GET;
    }

    return STATEMENT_UNKNOWN;
}


int execute_statement(Statement* stmt, Table* table) {
    switch (stmt->type) {
        case STATEMENT_INSERT:
            insert_event(&stmt->event, table);
            return 0;
        case STATEMENT_GET: {
            Event e;
            if (find_event_in_memory(stmt->query_id, table, &e) || read_event_by_id(stmt->query_id, &e)) {
                printf("%u: %s\n ⬑ Parents:", e.id, e.data);
                for (int i = 0; i < e.parent_count; i++) {
                    printf(" %u", e.parents[i]);
                }
                printf("\n");
            } else {
                printf("Event not found.\n");
            }
            return 0;
        }
        default:
            printf("Unrecognized statement type.\n");
            return 1;
    }
}