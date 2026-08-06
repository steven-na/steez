#pragma once

#include "common.h"
#include "smrt_arena.h"
#include "vec2.h"
#include "slidingwindow.h"

typedef struct {
    sliding_window_array_t  *xs;
    sliding_window_array_t  *ys;
                       u64 size;
} vec2d_sw_soa_t;

vec2d_sw_soa_t        vec2d_sw_soa(smrt_arena_t *arena, u64 size);
          void vec2d_sw_soa_insert(vec2d_sw_soa_t sw_vs, vec2d_t v);
   vec2d_soa_t    vec2d_sw_soa_get(vec2d_sw_soa_t sw_vs);
