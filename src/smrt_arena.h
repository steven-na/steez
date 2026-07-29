#pragma once

#include "common.h"

#define SMRT_ARENA_BASE_POS (sizeof(smrt_arena_t))

typedef struct {
    u64 reserve_size;
    u64 commit_size;

    u64 mark_pos;

    u64 pos;
    u64 commit_pos;

    b32 auto_decommit;
} smrt_arena_t;

smrt_arena_t     *smrt_arena_create(u64 reserve_size, u64 commit_size, b32 auto_decommit);
        void       *smrt_arena_push(smrt_arena_t *arena, u64 alloc_amount, b32 zero_out);
        void         smrt_arena_pop(smrt_arena_t *arena, u64 pop_amount);
        void      smrt_arena_pop_to(smrt_arena_t *arena, u64 location);
        void       smrt_arena_clear(smrt_arena_t *arena, b32 zero_out);
        void     smrt_arena_destroy(smrt_arena_t *arena);

        void        smrt_arena_mark(smrt_arena_t *arena);
        void  *smrt_arena_mark_push(smrt_arena_t *arena, u64 alloc_amount, b32 zero_out);
         b32 smrt_arena_pop_to_mark(smrt_arena_t *arena);

#define SMRTA_ALLOC_ARRAY(arena, T, n) (T*)smrt_arena_push(arena, sizeof(T) * (n), true)

typedef struct {
    smrt_arena_t *arena;
            u64   start_pos;
} smrta_temp_t;

smrta_temp_t smrta_temp_start(smrt_arena_t *arena);
        void   smrta_temp_end(smrta_temp_t temp);

smrta_temp_t smrta_scratch_start(smrt_arena_t **conflicts, u32 num_conflicts);
        void   smrta_scratch_end(smrta_temp_t temp);

#define SCRATCH_POOL_SIZE 2

#define TEMP_ARENA_PUSH(temp, T) (T*)smrt_arena_push(temp.arena, sizeof(T), true)
#define TEMP_ARENA_PUSH_ARRAY(temp, T, n) (T*)smrt_arena_push(temp.arena, sizeof(T) * (n), true)

u32 plat_get_pagesize(void);

void *plat_mem_reserve(u64 size);
 b32   plat_mem_commit(void *ptr, u64 size);
 b32 plat_mem_decommit(void *ptr, u64 size);
 b32  plat_mem_release(void *ptr, u64 size);
