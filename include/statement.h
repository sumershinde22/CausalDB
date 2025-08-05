#ifndef STATEMENT_H
#define STATEMENT_H

#include "event.h"

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_GET,
    STATEMENT_UNKNOWN
} StatementType;

typedef struct {
    StatementType type;
    Event event;       // For insert
    uint32_t query_id; // For get
} Statement;

StatementType parse_statement(const char* input, Statement* statement);
int execute_statement(Statement* stmt, Table* table);

#endif
