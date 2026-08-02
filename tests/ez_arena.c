#include "../src/ez_arena.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <criterion/internal/test.h>

Test(ez_arena, create_arena) {
    ez_arena_t *arena = ez_arena_create(KiB(1), true);

    cr_expect(arena->alloc_size == 1024);
    cr_expect(arena->pos == EZ_ARENA_BASE_POS);

    ez_arena_destroy(arena);
}

Test(ez_arena, alloc_arena) {
    ez_arena_t *arena = ez_arena_create(sizeof(uint32_t), true);

    uint32_t *i = ez_arena_alloc(arena, sizeof(uint32_t));

    *i = UINT32_MAX;

    cr_assert_eq(*i, UINT32_MAX);

    ez_arena_destroy(arena);
}

Test(ez_arena, alloc_zero_arena) {
    ez_arena_t *arena = ez_arena_create(sizeof(uint32_t), true);

    uint32_t *i = ez_arena_alloc(arena, sizeof(uint32_t));

    cr_assert_eq(*i, 0);

    ez_arena_destroy(arena);
}

Test(ez_arena, alloc_array_arena) {
    const size_t len = 24;
    cr_expect(len != 0);

    ez_arena_t *arena = ez_arena_create(sizeof(uint8_t) * len, true);

    uint8_t *arr = EZA_ALLOC_ARRAY(arena, uint8_t, len);

    for (size_t i = 0; i < len; i++) {
        arr[i] = UINT8_MAX / (i+1);
    }

    cr_expect_eq(arr[0], UINT8_MAX);
    cr_expect_eq(arr[len-1], UINT8_MAX / len);

    ez_arena_destroy(arena);
}

Test(ez_arena, alloc_clear_arena) {
    ez_arena_t *arena = ez_arena_create(sizeof(uint32_t), true);

    uint32_t *i = ez_arena_alloc(arena, sizeof(uint32_t));

    ez_arena_clear(arena, true);

    uint32_t *j = ez_arena_alloc(arena, sizeof(uint32_t));

    cr_assert_eq(i, j);

    ez_arena_destroy(arena);
}

Test(ez_arena, alloc_clear_zero_arena) {
    ez_arena_t *arena = ez_arena_create(sizeof(uint32_t), true);

    uint32_t *i = ez_arena_alloc(arena, sizeof(uint32_t));

    *i = UINT32_MAX;

    ez_arena_clear(arena, true);

    cr_expect_eq(*i, 0);

    uint32_t *j = ez_arena_alloc(arena, sizeof(uint32_t));

    cr_assert_eq(*j, 0);

    *j = UINT32_MAX;

    cr_expect_eq(*j, UINT32_MAX);
    cr_expect_eq(*i, UINT32_MAX);

    ez_arena_destroy(arena);
}

Test(ez_arena, alloc_clear_dont_zero_arena) {
    ez_arena_t *arena = ez_arena_create(sizeof(uint32_t), true);

    uint32_t *i = ez_arena_alloc(arena, sizeof(uint32_t));

    *i = UINT32_MAX;

    ez_arena_clear(arena, false);

    cr_expect_eq(*i, UINT32_MAX);

    ez_arena_destroy(arena);
}

Test(ez_arena, create_without_zeroing_is_usable) {
    ez_arena_t *arena = ez_arena_create(sizeof(uint32_t), false);

    uint32_t *i = ez_arena_alloc(arena, sizeof(uint32_t));
    *i = UINT32_MAX;

    cr_assert_eq(*i, UINT32_MAX);

    ez_arena_destroy(arena);
}

Test(ez_arena, create_fails_when_malloc_cannot_satisfy_size) {
    ez_arena_t *arena = ez_arena_create(UINT64_MAX - sizeof(ez_arena_t), true);

    cr_expect_eq(arena, NULL);
}

Test(ez_arena, alloc_nz_arena) {
    ez_arena_t *arena = ez_arena_create(sizeof(uint32_t), true);

    uint32_t *i = ez_arena_alloc_nz(arena, sizeof(uint32_t));
    *i = UINT32_MAX;

    cr_assert_eq(*i, UINT32_MAX);

    ez_arena_destroy(arena);
}

Test(ez_arena, alloc_exceeds_size_returns_null) {
    ez_arena_t *arena = ez_arena_create(sizeof(uint32_t), true);

    void *p = ez_arena_alloc(arena, KiB(1));

    cr_expect_eq(p, NULL);

    ez_arena_destroy(arena);
}

Test(ez_arena, alloc_nz_exceeds_size_returns_null) {
    ez_arena_t *arena = ez_arena_create(sizeof(uint32_t), true);

    void *p = ez_arena_alloc_nz(arena, KiB(1));

    cr_expect_eq(p, NULL);

    ez_arena_destroy(arena);
}
