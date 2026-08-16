#include "../src/smrt_arena.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <criterion/internal/test.h>

Test(smrt_arena, create_arena) {
    smrt_arena_t *arena = smrt_arena_create(KiB(1), plat_get_pagesize(), false);

    cr_expect(arena->reserve_size == MAX(KiB(1), plat_get_pagesize()));
    cr_expect(arena->pos == SMRT_ARENA_BASE_POS);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, alloc_arena) {
    smrt_arena_t *arena = smrt_arena_create(sizeof(uint32_t), plat_get_pagesize(), false);

    uint32_t *i = smrt_arena_push(arena, sizeof(uint32_t), true);

    *i = UINT32_MAX;

    cr_assert_eq(*i, UINT32_MAX);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, alloc_zero_arena) {
    smrt_arena_t *arena = smrt_arena_create(sizeof(uint32_t), plat_get_pagesize(), false);

    uint32_t *i = smrt_arena_push(arena, sizeof(uint32_t), true);

    cr_assert_eq(*i, 0);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, alloc_array_arena) {
    const size_t len = 24;
    cr_expect(len != 0);

    smrt_arena_t *arena = smrt_arena_create(sizeof(uint8_t) * len, plat_get_pagesize(), false);

    uint8_t *arr = SMRTA_ALLOC_ARRAY(arena, uint8_t, len);

    for (size_t i = 0; i < len; i++) {
        arr[i] = UINT8_MAX / (i+1);
    }

    cr_expect_eq(arr[0], UINT8_MAX);
    cr_expect_eq(arr[len-1], UINT8_MAX / len);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, alloc_clear_arena) {
    smrt_arena_t *arena = smrt_arena_create(sizeof(uint32_t), plat_get_pagesize(), false);

    uint32_t *i = smrt_arena_push(arena, sizeof(uint32_t), true);

    smrt_arena_clear(arena, true);

    uint32_t *j = smrt_arena_push(arena, sizeof(uint32_t), true);

    cr_assert_eq(i, j);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, alloc_clear_zero_arena) {
    smrt_arena_t *arena = smrt_arena_create(sizeof(uint32_t), plat_get_pagesize(), false);

    uint32_t *i = smrt_arena_push(arena, sizeof(uint32_t), true);

    *i = UINT32_MAX;

    smrt_arena_clear(arena, true);

    cr_expect_eq(*i, 0);

    uint32_t *j = smrt_arena_push(arena, sizeof(uint32_t), true);

    cr_assert_eq(*j, 0);

    *j = UINT32_MAX;

    cr_expect_eq(*j, UINT32_MAX);
    cr_expect_eq(*i, UINT32_MAX);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, alloc_clear_dont_zero_arena) {
    smrt_arena_t *arena = smrt_arena_create(sizeof(uint32_t), plat_get_pagesize(), false);

    uint32_t *i = smrt_arena_push(arena, sizeof(uint32_t), true);

    *i = UINT32_MAX;

    smrt_arena_clear(arena, false);

    cr_expect_eq(*i, UINT32_MAX);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, pop_arena) {
    smrt_arena_t *arena = smrt_arena_create(KiB(1), plat_get_pagesize(), false);

    uint64_t mark = arena->pos;
    uint32_t *i = smrt_arena_push(arena, sizeof(uint32_t), true);
    smrt_arena_push(arena, sizeof(uint32_t), true);

    smrt_arena_pop(arena, arena->pos - mark);
    uint32_t *j = smrt_arena_push(arena, sizeof(uint32_t), true);

    cr_assert_eq(i, j);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, pop_to_arena) {
    smrt_arena_t *arena = smrt_arena_create(KiB(1), plat_get_pagesize(), false);

    smrt_arena_push(arena, sizeof(uint32_t), true);
    uint64_t mark = arena->pos;
    smrt_arena_push(arena, sizeof(uint32_t), true);
    smrt_arena_push(arena, sizeof(uint32_t), true);

    smrt_arena_pop_to(arena, mark);

    cr_expect_eq(arena->pos, mark);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, pop_clamped_arena) {
    smrt_arena_t *arena = smrt_arena_create(KiB(1), plat_get_pagesize(), false);

    smrt_arena_push(arena, sizeof(uint32_t), true);
    smrt_arena_pop(arena, KiB(1));

    cr_expect_eq(arena->pos, SMRT_ARENA_BASE_POS);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, push_grows_commit_arena) {
    u32 pagesize = plat_get_pagesize();
    smrt_arena_t *arena = smrt_arena_create(KiB(64), pagesize, false);

    uint64_t initial_commit_pos = arena->commit_pos;
    uint8_t *arr = SMRTA_ALLOC_ARRAY(arena, uint8_t, pagesize * 2);
    arr[pagesize * 2 - 1] = UINT8_MAX;

    cr_expect_gt(arena->commit_pos, initial_commit_pos);
    cr_expect_eq(arr[pagesize * 2 - 1], UINT8_MAX);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, push_exceeds_reserve_arena) {
    smrt_arena_t *arena = smrt_arena_create(KiB(1), plat_get_pagesize(), false);

    void *p = smrt_arena_push(arena, KiB(64), true);

    cr_expect_eq(p, NULL);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, push_alignment_arena) {
    smrt_arena_t *arena = smrt_arena_create(KiB(1), plat_get_pagesize(), false);

    smrt_arena_push(arena, sizeof(uint8_t), true);
    void *p = smrt_arena_push(arena, sizeof(uint64_t), true);

    cr_expect_eq((uintptr_t)p % ARENA_ALIGN, 0);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, create_fails_when_reserve_exceeds_address_space) {
    smrt_arena_t *arena = smrt_arena_create((u64)1 << 48, plat_get_pagesize(), false);

    cr_expect_eq(arena, NULL);
}

Test(smrt_arena, mark_push_sets_mark) {
    smrt_arena_t *arena = smrt_arena_create(KiB(1), plat_get_pagesize(), false);

    uint64_t pre_mark_pos = arena->pos;
    smrt_arena_mark_push(arena, sizeof(uint32_t), true);

    cr_expect_eq(arena->mark_pos, pre_mark_pos);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, mark_push_failure_leaves_mark_unset) {
    smrt_arena_t *arena = smrt_arena_create(KiB(1), plat_get_pagesize(), false);

    void *p = smrt_arena_mark_push(arena, KiB(64), true);

    cr_expect_eq(p, NULL);
    cr_expect_eq(arena->mark_pos, 0);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, pop_to_mark_restores_position) {
    smrt_arena_t *arena = smrt_arena_create(KiB(1), plat_get_pagesize(), false);

    uint32_t *i = smrt_arena_mark_push(arena, sizeof(uint32_t), true);
    smrt_arena_push(arena, sizeof(uint32_t), true);
    smrt_arena_push(arena, sizeof(uint32_t), true);

    b32 popped = smrt_arena_pop_to_mark(arena);
    uint32_t *j = smrt_arena_push(arena, sizeof(uint32_t), true);

    cr_expect_eq(popped, 0);
    cr_assert_eq(i, j);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, mark_at_base_position_is_valid) {
    smrt_arena_t *arena = smrt_arena_create(KiB(1), plat_get_pagesize(), false);

    cr_assert_eq(arena->pos, SMRT_ARENA_BASE_POS);

    smrt_arena_mark_push(arena, sizeof(uint32_t), true);

    cr_expect_eq(arena->mark_pos, SMRT_ARENA_BASE_POS);

    b32 popped = smrt_arena_pop_to_mark(arena);

    cr_expect_eq(popped, 0);
    cr_expect_eq(arena->pos, SMRT_ARENA_BASE_POS);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, pop_to_mark_without_mark_fails) {
    smrt_arena_t *arena = smrt_arena_create(KiB(1), plat_get_pagesize(), false);

    smrt_arena_push(arena, sizeof(uint32_t), true);

    b32 popped = smrt_arena_pop_to_mark(arena);

    cr_expect_eq(popped, -1);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, pop_to_ahead_of_pos_is_noop) {
    smrt_arena_t *arena = smrt_arena_create(KiB(1), plat_get_pagesize(), false);

    smrt_arena_push(arena, sizeof(uint32_t), true);
    uint64_t pos = arena->pos;

    smrt_arena_pop_to(arena, pos + KiB(1));

    cr_expect_eq(arena->pos, pos);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, auto_decommit_shrinks_on_pop) {
    u32 pagesize = plat_get_pagesize();
    smrt_arena_t *arena = smrt_arena_create(KiB(64), pagesize, true);

    SMRTA_ALLOC_ARRAY(arena, uint8_t, pagesize * 3);
    cr_assert_eq(arena->commit_pos, pagesize * 4);

    smrt_arena_pop(arena, pagesize * 2);

    cr_expect_eq(arena->commit_pos, pagesize * 2);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, auto_decommit_noop_within_same_chunk) {
    u32 pagesize = plat_get_pagesize();
    smrt_arena_t *arena = smrt_arena_create(KiB(64), pagesize, true);

    SMRTA_ALLOC_ARRAY(arena, uint8_t, pagesize / 2);
    uint64_t commit_pos_before = arena->commit_pos;

    smrt_arena_pop(arena, sizeof(uint8_t));

    cr_expect_eq(arena->commit_pos, commit_pos_before);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, auto_decommit_clear_leaves_one_chunk) {
    u32 pagesize = plat_get_pagesize();
    smrt_arena_t *arena = smrt_arena_create(KiB(64), pagesize, true);

    SMRTA_ALLOC_ARRAY(arena, uint8_t, pagesize * 3);
    cr_assert_gt(arena->commit_pos, (u64)pagesize);

    smrt_arena_clear(arena, false);

    cr_expect_eq(arena->commit_pos, pagesize);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, auto_decommit_off_by_default_preserves_commit) {
    u32 pagesize = plat_get_pagesize();
    smrt_arena_t *arena = smrt_arena_create(KiB(64), pagesize, false);

    SMRTA_ALLOC_ARRAY(arena, uint8_t, pagesize * 3);
    uint64_t commit_pos_before = arena->commit_pos;

    smrt_arena_pop(arena, pagesize * 2);

    cr_expect_eq(arena->commit_pos, commit_pos_before);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, auto_decommit_pop_past_mark_resets_mark) {
    u32 pagesize = plat_get_pagesize();
    smrt_arena_t *arena = smrt_arena_create(KiB(64), pagesize, true);

    SMRTA_ALLOC_ARRAY(arena, uint8_t, pagesize);
    smrt_arena_mark_push(arena, pagesize, true);
    SMRTA_ALLOC_ARRAY(arena, uint8_t, pagesize);

    uint64_t mark_pos = arena->mark_pos;
    cr_assert_gt(mark_pos, (u64)0);

    smrt_arena_pop(arena, pagesize * 3);

    cr_expect_lt(arena->commit_pos, mark_pos);
    cr_expect_eq(arena->mark_pos, 0);

    i32 popped = smrt_arena_pop_to_mark(arena);
    cr_expect_eq(popped, -1);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, mark_sets_current_pos) {
    smrt_arena_t *arena = smrt_arena_create(KiB(1), plat_get_pagesize(), false);

    smrt_arena_push(arena, sizeof(uint32_t), true);
    uint64_t pos = arena->pos;

    smrt_arena_mark(arena);

    cr_expect_eq(arena->mark_pos, pos);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, temp_start_captures_arena_and_pos) {
    smrt_arena_t *arena = smrt_arena_create(KiB(1), plat_get_pagesize(), false);
    smrt_arena_push(arena, sizeof(uint32_t), true);

    smrta_temp_t temp = smrta_temp_start(arena);

    cr_expect_eq(temp.arena, arena);
    cr_expect_eq(temp.start_pos, arena->pos);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, temp_start_end_restores_position) {
    smrt_arena_t *arena = smrt_arena_create(KiB(1), plat_get_pagesize(), false);

    smrt_arena_push(arena, sizeof(uint32_t), true);
    uint64_t pos_before = arena->pos;

    smrta_temp_t temp = smrta_temp_start(arena);
    smrt_arena_push(arena, sizeof(uint32_t), true);
    smrt_arena_push(arena, sizeof(uint32_t), true);

    smrta_temp_end(temp);

    cr_expect_eq(arena->pos, pos_before);

    smrt_arena_destroy(arena);
}

Test(smrt_arena, scratch_start_end_provides_usable_arena) {
    smrta_temp_t scratch = smrta_scratch_start(NULL, 0);

    cr_assert_neq(scratch.arena, NULL);

    uint32_t *i = TEMP_ARENA_PUSH(scratch, uint32_t);
    *i = UINT32_MAX;

    cr_expect_eq(*i, UINT32_MAX);

    smrta_scratch_end(scratch);
}

Test(smrt_arena, scratch_end_pops_arena_to_start) {
    smrta_temp_t scratch = smrta_scratch_start(NULL, 0);
    uint64_t pos_before = scratch.arena->pos;

    TEMP_ARENA_PUSH(scratch, uint32_t);
    cr_assert_gt(scratch.arena->pos, pos_before);

    smrta_scratch_end(scratch);

    cr_expect_eq(scratch.arena->pos, pos_before);
}

Test(smrt_arena, scratch_start_avoids_conflicts) {
    smrta_temp_t first = smrta_scratch_start(NULL, 0);
    cr_assert_neq(first.arena, NULL);

    smrt_arena_t *conflicts[] = { first.arena };
    smrta_temp_t second = smrta_scratch_start(conflicts, 1);

    cr_assert_neq(second.arena, NULL);
    cr_expect_neq(second.arena, first.arena);

    smrta_scratch_end(second);
    smrta_scratch_end(first);
}

Test(smrt_arena, scratch_start_returns_zeroed_when_pool_exhausted) {
    smrta_temp_t a = smrta_scratch_start(NULL, 0);

    smrt_arena_t *a_conflict[] = { a.arena };
    smrta_temp_t b = smrta_scratch_start(a_conflict, 1);
    cr_assert_neq(b.arena, a.arena);

    smrt_arena_t *conflicts[] = { a.arena, b.arena };
    smrta_temp_t c = smrta_scratch_start(conflicts, 2);

    cr_expect_eq(c.arena, NULL);

    smrta_scratch_end(b);
    smrta_scratch_end(a);
}

Test(smrt_arena, decommit_then_recommit_pages) {
    u32 pagesize = plat_get_pagesize();
    void *mem = plat_mem_reserve(pagesize);
    cr_assert_neq(mem, NULL);

    cr_assert_eq(plat_mem_commit(mem, pagesize), true);

    uint8_t *bytes = (uint8_t*)mem;
    bytes[0] = UINT8_MAX;

    cr_expect_eq(plat_mem_decommit(mem, pagesize), true);
    cr_expect_eq(plat_mem_commit(mem, pagesize), true);

    cr_expect_eq(bytes[0], 0);

    plat_mem_release(mem, pagesize);
}
