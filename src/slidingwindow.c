#include "slidingwindow.h"
#include "common.h"
#include "smrt_arena.h"
#include "log.h"

#include <string.h>

sliding_window_array_t *sw_create(smrt_arena_t *arena, u64 elem_size, u64 size) {
    sliding_window_array_t *sw = smrt_arena_push(arena, sizeof(sliding_window_array_t) + 2 * elem_size * size, true);
    sw->size = size;
    sw->element_size_bytes = elem_size;

    #ifndef NLOG_TRACE
        log_trace("Created sliding window; elem size %lu, capacity %lu", elem_size, size);
    #endif /* ifndef NLOG_TRACE */

    return sw;
}

void sw_insert(sliding_window_array_t *sw, void *x) {
    if (sw->num_elements == sw->size) {
        sw->start_idx++;
    }

    if ((sw->start_idx + sw->size) > (2 * sw->size)) {
        u64 src_offset = SLIDING_WINDOW_BASE_POS + (sw->start_idx * sw->element_size_bytes);
        u64 bytes_to_copy = (sw->size - 1) * sw->element_size_bytes;
        memmove(
            ((u8*)sw + SLIDING_WINDOW_BASE_POS),
            ((u8*)sw + src_offset),
            bytes_to_copy
        );

        sw->start_idx = 0;
    }

    u64 new_pos = sw->start_idx + sw->num_elements;

    if (new_pos >= sw->start_idx + sw->size) {
        new_pos = sw->start_idx + sw->size - 1;
    }

    memcpy(((u8*)sw + SLIDING_WINDOW_BASE_POS + (new_pos * sw->element_size_bytes)), x, sw->element_size_bytes);

    if (sw->num_elements < sw->size) {
        sw->num_elements++;
    }
}
