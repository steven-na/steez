#pragma once

#include "common.h"

#include <assert.h>
#include <math.h>

typedef union {
    struct { f64 x, y; };
    f64 d[2];
} vec2d_t;

typedef struct {
    f64 *xs;
    f64 *ys;
    u64 size;
} vec2d_soa_t;

#define VEC2D_ZERO (vec2d_t){.x=0.0, .y=0.0}
#define VEC2D_UNIF(n) (vec2d_t){.x=(n), .y=(n)}
#define VEC2D_FROM(ix, iy) (vec2d_t){.x=(ix), .y=(iy)}

   void   vec2d_soa_add_n(vec2d_soa_t       *restrict out ,
                          vec2d_soa_t const *restrict a   ,
                          vec2d_soa_t const *restrict b   );

   void   vec2d_soa_sub_n(vec2d_soa_t       *restrict out ,
                          vec2d_soa_t const *restrict lhs ,
                          vec2d_soa_t const *restrict rhs );

   void vec2d_soa_scale_n(vec2d_soa_t       *restrict out ,
                          vec2d_soa_t const *restrict in  ,
                                  f64       scalar        );

   void  vec2d_soa_norm_n(vec2d_soa_t       *restrict out ,
                          vec2d_soa_t const *restrict in  );

vec2d_t vec2d_soa_average(vec2d_soa_t const *vs);

// -------------------------------------------
// vec2d_t inline operations
// -------------------------------------------

static inline
b8 vec2d_epsilon_eq(vec2d_t v1, vec2d_t v2, f64 epsilon) {
    return F64_EQ(v1.x, v2.x, epsilon) && F64_EQ(v1.y, v2.y, epsilon);
}

static inline
vec2d_t vec2d_add(vec2d_t v1, vec2d_t v2) {
    return (vec2d_t){
        .x = v1.x + v2.x,
        .y = v1.y + v2.y,
    };
}

static inline
vec2d_t vec2d_sub(vec2d_t lhs, vec2d_t rhs) {
    return (vec2d_t){
        .x = lhs.x - rhs.x,
        .y = lhs.y - rhs.y,
    };
}

static inline
vec2d_t vec2d_scale(vec2d_t v, f64 scalar) {
    return (vec2d_t){
        .x = v.x * scalar,
        .y = v.y * scalar,
    };
}

static inline
f64 vec2d_length_squared(vec2d_t v) {
    return v.x * v.x + v.y * v.y;
}

static inline
f64 vec2d_length(vec2d_t v) {
    return sqrt(vec2d_length_squared(v));
}

static inline
vec2d_t vec2d_normalize(vec2d_t v) {
    f64 lsquared = vec2d_length_squared(v);

    if (F64_EQ(lsquared, 0.0, 1e-9)) {
        return VEC2D_ZERO;
    }

    return vec2d_scale(v, 1.0 / sqrt(lsquared));
}

static inline
f64 vec2d_dot(vec2d_t v1, vec2d_t v2) {
    return (v1.x * v2.x) + (v1.y * v2.y);
}

// -------------------------------------------
// vec2d_soa_t inline operations
// -------------------------------------------

static inline
vec2d_t vec2d_soa_get(const vec2d_soa_t *vs, u64 i) {
    assert(i < vs->size && "Input index >= vs.size");
    return (vec2d_t){
        .x = vs->xs[i],
        .y = vs->ys[i],
    };
}

static inline
void vec2d_soa_set(vec2d_soa_t *vs, u64 i, vec2d_t p) {
    assert(i < vs->size && "Input index >= vs.size");
    vs->xs[i] = p.x;
    vs->ys[i] = p.y;
}

