#include "../src/vec2.h"

#include <math.h>
#include <stddef.h>

#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <criterion/internal/test.h>

Test(vec2d, zeroed_vec) {
    vec2d_t v = {.x = 0.0, .y = 0.0};
    vec2d_t z = VEC2D_ZERO;

    cr_expect_eq(v.x, z.x);
    cr_expect_eq(v.y, z.y);
}

Test(vec2d, uniform_vec) {
    vec2d_t v = {.x = 3.0, .y = 3.0};
    vec2d_t u = VEC2D_UNIF(3.0);

    cr_expect_eq(v.x, u.x);
    cr_expect_eq(v.y, u.y);
}

Test(vec2d, from_literals) {
    vec2d_t v = {.x = 3.0, .y = 4.0};
    vec2d_t l = VEC2D_FROM(3.0, 4.0);

    cr_expect_eq(v.x, l.x);
    cr_expect_eq(v.y, l.y);
}

Test(vec2d, zeroed_eps_eq) {
    vec2d_t v = {.x = 0.0, .y = 0.0};
    vec2d_t z = VEC2D_ZERO;

    cr_expect(vec2d_epsilon_eq(v, z, 1e-9));
}

Test(vec2d, eps_neq) {
    vec2d_t v = {.x = 0.0, .y = 0.0};
    vec2d_t n = VEC2D_FROM(3.0, 10.0);

    cr_expect(!vec2d_epsilon_eq(v, n, 1e-9));
}

Test(vec2d, vec_add) {
    vec2d_t first = VEC2D_FROM(10.0, 20.0);
    vec2d_t second = VEC2D_FROM(100.0, 25.0);

    vec2d_t opd = vec2d_add(first, second);
    vec2d_t expected = VEC2D_FROM(110.0, 45.0);

    cr_expect(vec2d_epsilon_eq(opd, expected, 1e-9));
}

Test(vec2d, vec_sub) {
    vec2d_t first = VEC2D_FROM(10.0, 20.0);
    vec2d_t second = VEC2D_FROM(100.0, 25.0);

    vec2d_t opd = vec2d_sub(first, second);
    vec2d_t expected = VEC2D_FROM(-90.0, -5.0);

    cr_expect(vec2d_epsilon_eq(opd, expected, 1e-9));
}

Test(vec2d, vec_scale) {
    vec2d_t first = VEC2D_FROM(10.0, 20.0);

    vec2d_t opd = vec2d_scale(first, 10.0);
    vec2d_t expected = VEC2D_FROM(100.0, 200.0);

    cr_expect(vec2d_epsilon_eq(opd, expected, 1e-9));
}

Test(vec2d, vec_length_squared) {
    vec2d_t first = VEC2D_FROM(10.0, 0.0);

    f64 opd = vec2d_length_squared(first);
    f64 expected = 100.0;

    cr_expect(F64_EQ(opd, expected, 1e-9));
}

Test(vec2d, vec_length) {
    vec2d_t first = VEC2D_FROM(0.0, 15.0);

    f64 opd = vec2d_length(first);
    f64 expected = 15.0;

    cr_expect(F64_EQ(opd, expected, 1e-9));
}

Test(vec2d, vec_norm1) {
    vec2d_t first = VEC2D_FROM(10.0, 0.0);

    vec2d_t opd = vec2d_normalize(first);
    vec2d_t expected = VEC2D_FROM(1.0, 0);

    cr_expect(vec2d_epsilon_eq(opd, expected, 1e-9));
}

Test(vec2d, vec_norm2) {
    vec2d_t first = VEC2D_FROM(0.0, 10.0);

    vec2d_t opd = vec2d_normalize(first);
    vec2d_t expected = VEC2D_FROM(0.0, 1.0);

    cr_expect(vec2d_epsilon_eq(opd, expected, 1e-9));
}

Test(vec2d, vec_norm3) {
    vec2d_t first = VEC2D_FROM(1.0, 1.0);

    vec2d_t opd = vec2d_normalize(first);
    vec2d_t expected = VEC2D_FROM(1.0 / sqrt(2.0), 1.0 / sqrt(2.0));

    cr_expect(vec2d_epsilon_eq(opd, expected, 1e-9));
}

Test(vec2d, vec_dot) {
    vec2d_t first = VEC2D_FROM(1.0, 2.0);
    vec2d_t second = VEC2D_FROM(3.0, 4.0);

    f64 opd = vec2d_dot(first, second);
    f64 expected = 11.0;

    cr_expect(F64_EQ(opd, expected, 1e-9));
}

Test(vec2d_soa, add_n) {
    vec2d_soa_t a = { .xs = (f64[]){1.0, 2.0, 3.0}, .ys = (f64[]){4.0, 5.0, 6.0}, .size = 3 };
    vec2d_soa_t b = { .xs = (f64[]){10.0, 20.0, 30.0}, .ys = (f64[]){40.0, 50.0, 60.0}, .size = 3 };

    f64 out_x[3], out_y[3];
    vec2d_soa_t out = { .xs = out_x, .ys = out_y, .size = 3 };

    vec2d_soa_add_n(&out, &a, &b);

    for (size_t i = 0; i < 3; i++) {
        cr_expect(F64_EQ(out.xs[i], a.xs[i] + b.xs[i], 1e-9));
        cr_expect(F64_EQ(out.ys[i], a.ys[i] + b.ys[i], 1e-9));
    }
}

Test(vec2d_soa, sub_n) {
    vec2d_soa_t lhs = { .xs = (f64[]){10.0, 20.0, 30.0}, .ys = (f64[]){40.0, 50.0, 60.0}, .size = 3 };
    vec2d_soa_t rhs = { .xs = (f64[]){1.0, 2.0, 3.0}, .ys = (f64[]){4.0, 5.0, 6.0}, .size = 3 };

    f64 out_x[3], out_y[3];
    vec2d_soa_t out = { .xs = out_x, .ys = out_y, .size = 3 };

    vec2d_soa_sub_n(&out, &lhs, &rhs);

    for (size_t i = 0; i < 3; i++) {
        cr_expect(F64_EQ(out.xs[i], lhs.xs[i] - rhs.xs[i], 1e-9));
        cr_expect(F64_EQ(out.ys[i], lhs.ys[i] - rhs.ys[i], 1e-9));
    }
}

Test(vec2d_soa, scale_n) {
    vec2d_soa_t in = { .xs = (f64[]){1.0, 2.0, 3.0}, .ys = (f64[]){4.0, 5.0, 6.0}, .size = 3 };

    f64 out_x[3], out_y[3];
    vec2d_soa_t out = { .xs = out_x, .ys = out_y, .size = 3 };

    vec2d_soa_scale_n(&out, &in, 10.0);

    for (size_t i = 0; i < 3; i++) {
        cr_expect(F64_EQ(out.xs[i], in.xs[i] * 10.0, 1e-9));
        cr_expect(F64_EQ(out.ys[i], in.ys[i] * 10.0, 1e-9));
    }
}

Test(vec2d_soa, norm_n_handles_zero_and_nonzero) {
    vec2d_soa_t in = { .xs = (f64[]){3.0, 0.0}, .ys = (f64[]){4.0, 0.0}, .size = 2 };

    f64 out_x[2], out_y[2];
    vec2d_soa_t out = { .xs = out_x, .ys = out_y, .size = 2 };

    vec2d_soa_norm_n(&out, &in);

    cr_expect(F64_EQ(out.xs[0], 0.6, 1e-9));
    cr_expect(F64_EQ(out.ys[0], 0.8, 1e-9));
    cr_expect(F64_EQ(out.xs[1], 0.0, 1e-9));
    cr_expect(F64_EQ(out.ys[1], 0.0, 1e-9));
}

Test(vec2d_soa, average) {
    vec2d_soa_t vs = { .xs = (f64[]){2.0, 4.0, 6.0}, .ys = (f64[]){10.0, 20.0, 30.0}, .size = 3 };

    vec2d_t avg = vec2d_soa_average(&vs);

    cr_expect(F64_EQ(avg.x, 4.0, 1e-9));
    cr_expect(F64_EQ(avg.y, 20.0, 1e-9));
}
