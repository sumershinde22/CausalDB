#ifndef DB_H
#define DB_H

#include "event.h"

void open_db(const char* filename);
void close_db();

void insert_event(Event* e, Table* table);
int find_event_in_memory(uint32_t id, Table* table, Event* out);
int read_event_by_id(uint32_t id, Event* out);
Table* load_table(const char* filename);

#endif
