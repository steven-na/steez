#pragma once

#include "common.h"

#define EZ_ARENA_BASE_POS (sizeof(ez_arena_t))

typedef struct {
    u64 alloc_size;
    u64        pos;
} ez_arena_t;

ez_arena_t *  ez_arena_create(u64 size, b32 zero_out);

      void *   ez_arena_alloc(ez_arena_t *arena, u64 alloc_amount);
      void *ez_arena_alloc_nz(ez_arena_t *arena, u64 alloc_amount);
      void     ez_arena_clear(ez_arena_t *arena, b32 zero_out);
      void   ez_arena_destroy(ez_arena_t *arena);

#define EZA_ALLOC_ARRAY(arena, T, n) (T*)ez_arena_alloc(arena, sizeof(T) * (n))
