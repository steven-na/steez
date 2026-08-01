#include "vec2.h"
#include "common.h"

#include <assert.h>
#include <math.h>

void vec2d_soa_add_n(vec2d_soa_t *restrict out, const vec2d_soa_t *restrict a, const vec2d_soa_t *restrict b) {
    assert(out->size > 0        && "Out should have size > 0"             );
    assert(out->size == a->size && "Input should have same size as output");
    assert(out->size == b->size && "Input should have same size as output");

    f64       *restrict out_x = out->xs;
    f64       *restrict out_y = out->ys;
    f64 const *restrict   a_x =   a->xs;
    f64 const *restrict   a_y =   a->ys;
    f64 const *restrict   b_x =   b->xs;
    f64 const *restrict   b_y =   b->ys;

    for (u64 i = 0; i < out->size; i++) out_x[i] = a_x[i] + b_x[i];
    for (u64 i = 0; i < out->size; i++) out_y[i] = a_y[i] + b_y[i];
}

void vec2d_soa_sub_n(vec2d_soa_t *restrict out, const vec2d_soa_t *restrict lhs, const vec2d_soa_t *restrict rhs) {
    assert(out->size > 0          && "Out should have size > 0"             );
    assert(out->size == lhs->size && "Input should have same size as output");
    assert(out->size == rhs->size && "Input should have same size as output");

    f64       *restrict out_x = out->xs;
    f64       *restrict out_y = out->ys;
    f64 const *restrict lhs_x = lhs->xs;
    f64 const *restrict lhs_y = lhs->ys;
    f64 const *restrict rhs_x = rhs->xs;
    f64 const *restrict rhs_y = rhs->ys;

    for (u64 i = 0; i < out->size; i++) out_x[i] = lhs_x[i] - rhs_x[i];
    for (u64 i = 0; i < out->size; i++) out_y[i] = lhs_y[i] - rhs_y[i];
}

void vec2d_soa_scale_n(vec2d_soa_t *restrict out, const vec2d_soa_t *restrict in, f64 scalar) {
    assert(out->size > 0         && "Out should have size > 0"             );
    assert(out->size == in->size && "Input should have same size as output");

    f64       *restrict out_x = out->xs;
    f64       *restrict out_y = out->ys;
    f64 const *restrict  in_x =  in->xs;
    f64 const *restrict  in_y =  in->ys;

    for (u64 i = 0; i < out->size; i++) out_x[i] = in_x[i] * scalar;
    for (u64 i = 0; i < out->size; i++) out_y[i] = in_y[i] * scalar;
}

void vec2d_soa_norm_n(vec2d_soa_t *restrict out, vec2d_soa_t const *restrict in) {
    assert(out->size > 0         && "Out should have size > 0"             );
    assert(out->size == in->size && "Input should have same size as output");

    f64       *restrict out_x = out->xs;
    f64       *restrict out_y = out->ys;
    f64 const *restrict  in_x =  in->xs;
    f64 const *restrict  in_y =  in->ys;

    for (u64 i = 0; i < out->size; i++) {
        f64 lsquared = (in_x[i] * in_x[i]) +
                       (in_y[i] * in_y[i]);

        if (F64_EQ(lsquared, 0.0, 1e-9)) { out_x[i] = 0.0;
                                           out_y[i] = 0.0;
                                           continue; }
        f64 mag = sqrt(lsquared);

        out_x[i] = in_x[i] / mag;
        out_y[i] = in_y[i] / mag;
    }
}

vec2d_t vec2d_soa_average(const vec2d_soa_t *vs) {
    assert(vs->size != 0 && "Input should have size > 0");

    f64 sum_x = 0.0, sum_y = 0.0;
    for (u64 i = 0; i < vs->size; i++) {
        sum_x += vs->xs[i];
        sum_y += vs->ys[i];
    }

    return vec2d_scale(VEC2D_FROM(sum_x, sum_y), 1.0 / vs->size);
}
