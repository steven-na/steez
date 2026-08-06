#pragma once

#include "common.h"
#include "smrt_arena.h"

#define SLIDING_WINDOW_BASE_POS (sizeof(sliding_window_array_t))

typedef struct {
     u64               size;
     u64       num_elements;
     u64          start_idx;
     u64 element_size_bytes;
} sliding_window_array_t;

#define SMRTA_ALLOC_SLIDINGWINDOW(arena, T, n) (sliding_window_array_t*)sw_create(arena, sizeof(T), (n))
#define SLIDINGWINDOW_GET(w, T) ((T*)((u8*)(w) + SLIDING_WINDOW_BASE_POS) + (w)->start_idx)

sliding_window_array_t *sw_create(smrt_arena_t *arena, u64 elem_size, u64 size);
void sw_insert(sliding_window_array_t *sw, void *x);
