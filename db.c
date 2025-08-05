#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



FILE* db_file;

Table* load_table(const char* filename) {
    open_db(filename);
    Table* table = malloc(sizeof(Table));
    table->num_events = 0;

    Event e;
    size_t i = 0;
    while (read_event_by_id(i + 1, &e)) {
        Event* dest = event_slot(table, table->num_events);
        *dest = e;
        table->num_events++;

        i++;
    }
    return table;
}

void open_db(const char* filename) {
    db_file = fopen(filename, "a+b");
    if (!db_file) {
        perror("fopen");
        exit(1);
    }
}

void close_db() {
    fclose(db_file);
}

void insert_event(Event* e, Table* table) {
    if (table->num_events >= TABLE_MAX_EVENTS) {
        printf("Error: table full.\n");
        return;
    }

    table->events[table->num_events++] = *e;

    uint8_t buffer[ROW_SIZE];
    serialize_event(e, buffer);
    fseek(db_file, 0, SEEK_END);
    fwrite(buffer, ROW_SIZE, 1, db_file);
    fflush(db_file);
}

int read_event_by_id(uint32_t id, Event* out) {
    fseek(db_file, 0, SEEK_SET);
    uint8_t buffer[ROW_SIZE];
    while (fread(buffer, ROW_SIZE, 1, db_file)) {
        deserialize_event(buffer, out);
        if (out->id == id) return 1;
    }
    return 0;
}

int find_event_in_memory(uint32_t id, Table* table, Event* out) {
    for (size_t i = 0; i < table->num_events; i++) {
        if (table->events[i].id == id) {
            *out = table->events[i];
            return 1;
        }
    }
    return 0;
}
