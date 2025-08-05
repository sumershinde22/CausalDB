#include "event.h"
#include <stdio.h>
#include <string.h>

void serialize_event(Event* src, void* dest) {
    memcpy(dest, &src->id, 4);
    memcpy(dest + 4, &src->parent_count, 1);
    memcpy(dest + 5, &src->parents, 4 * MAX_PARENTS);
    memcpy(dest + 37, &src->data, MAX_DATA_LENGTH);
}

void deserialize_event(void* src, Event* dest) {
    memcpy(&dest->id, src, 4);
    memcpy(&dest->parent_count, src + 4, 1);
    memcpy(&dest->parents, src + 5, 4 * MAX_PARENTS);
    memcpy(&dest->data, src + 37, MAX_DATA_LENGTH);
}
