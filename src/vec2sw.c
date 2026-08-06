#include "vec2sw.h"
#include "slidingwindow.h"
#include "vec2.h"

vec2d_sw_soa_t vec2d_sw_soa(smrt_arena_t *arena, u64 size) {
    vec2d_sw_soa_t vs = { .size=size };
    vs.xs = SMRTA_ALLOC_SLIDINGWINDOW(arena, f64, size);
    vs.ys = SMRTA_ALLOC_SLIDINGWINDOW(arena, f64, size);
    return vs;
}

void vec2d_sw_soa_insert(vec2d_sw_soa_t sw_vs, vec2d_t v) {
    sw_insert(sw_vs.xs, (void*)&v.x);
    sw_insert(sw_vs.ys, (void*)&v.y);
}

vec2d_soa_t vec2d_sw_soa_get(vec2d_sw_soa_t sw_vs) {
    return (vec2d_soa_t){
        .size = sw_vs.xs->num_elements,
        .xs = SLIDINGWINDOW_GET(sw_vs.xs, f64),
        .ys = SLIDINGWINDOW_GET(sw_vs.ys, f64),
    };
}
