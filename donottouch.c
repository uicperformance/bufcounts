#include "donottouch.h"

void update_buffer(buf *buffer, unsigned thread_id) {
    buffer->array[thread_id]++;
}

int check_buffer_alignment(buf *buffer) {
    return ((long unsigned)buffer % alignof(buf));
}
