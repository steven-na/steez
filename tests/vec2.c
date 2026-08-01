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
