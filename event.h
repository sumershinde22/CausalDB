#ifndef EVENT_H
#define EVENT_H

#include <stddef.h>
#include <stdint.h>

#define MAX_PARENTS 8
#define MAX_DATA_LENGTH 128
#define ROW_SIZE 168

typedef struct {
    uint32_t id;
    uint8_t parent_count;
    uint32_t parents[MAX_PARENTS];
    char data[MAX_DATA_LENGTH];
} Event;

#define TABLE_MAX_EVENTS 1000

typedef struct {
    Event events[TABLE_MAX_EVENTS];
    size_t num_events;
} Table;

static inline Event* event_slot(Table* table, size_t row_num) {
    return &table->events[row_num];
}

// 🔽 ADD THESE LINES:
void serialize_event(Event* src, void* dest);
void deserialize_event(void* src, Event* dest);



#endif
