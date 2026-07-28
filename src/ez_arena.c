#include "ez_arena.h"

#include <stdlib.h>
#include <string.h>

ez_arena_t *ez_arena_create(u64 size, b32 zero_out) {
    ez_arena_t *alloc = malloc(size + sizeof(ez_arena_t));

    if (!alloc) {
        return NULL;
    }

    if (zero_out) {
        memset((u8*)alloc+EZ_ARENA_BASE_POS, 0, size);
    }

    *alloc = (ez_arena_t){ .alloc_size = size, .pos = EZ_ARENA_BASE_POS };

    return alloc;
}

void *ez_arena_alloc(ez_arena_t *arena, u64 alloc_amount) {
    u64 pos_aligned = ALIGN_UP_POW2(arena->pos, ARENA_ALIGN);
    u64 new_pos = pos_aligned + alloc_amount;

    if (new_pos > arena->alloc_size + sizeof(ez_arena_t)) { return NULL; }

    arena->pos = new_pos;

    u8* out = (u8*)arena + pos_aligned;

    memset(out, 0, alloc_amount);

    return out;
}

void *ez_arena_alloc_nz(ez_arena_t *arena, u64 alloc_amount) {
    u64 pos_aligned = ALIGN_UP_POW2(arena->pos, ARENA_ALIGN);
    u64 new_pos = pos_aligned + alloc_amount;

    if (new_pos > arena->alloc_size + sizeof(ez_arena_t)) { return NULL; }

    arena->pos = new_pos;

    return (u8*)arena + pos_aligned;
}

void ez_arena_clear(ez_arena_t *arena, b32 zero_out) {
    if (zero_out) {
        memset((u8*)arena+EZ_ARENA_BASE_POS, 0, arena->alloc_size);
    }

    arena->pos = EZ_ARENA_BASE_POS;
}

void ez_arena_destroy(ez_arena_t *arena) {
    free(arena);
}
