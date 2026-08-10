#if defined(__linux__)
    #ifndef _DEFAULT_SOURCE
        #define _DEFAULT_SOURCE
    #endif /* ifndef _DEFAULT_SOURCE */
#endif

#include "common.h"
#include "smrt_arena.h"

#include <string.h>

static __thread smrt_arena_t *_scratch_pool[SCRATCH_POOL_SIZE] = { 0 };

smrt_arena_t *smrt_arena_create(u64 reserve_size, u64 commit_size, b32 auto_decommit) {
    u32 pagesize = plat_get_pagesize();

    reserve_size = ALIGN_UP_POW2(reserve_size + sizeof(smrt_arena_t), pagesize);
     commit_size = ALIGN_UP_POW2( commit_size, pagesize);

    smrt_arena_t *arena = plat_mem_reserve(reserve_size);

    if (!plat_mem_commit(arena, commit_size)) {
        return NULL;
    }

    arena->reserve_size = reserve_size;
     arena->commit_size = commit_size;
      arena->commit_pos = commit_size;
             arena->pos = SMRT_ARENA_BASE_POS;
        arena->mark_pos = 0;
    arena->auto_decommit = auto_decommit;

    return arena;

}

static void smrt_arena__shrink_commit(smrt_arena_t *arena) {
    if (!arena->auto_decommit) { return; }

    u64 target_commit_pos = arena->pos + arena->commit_size - 1;
    target_commit_pos -= target_commit_pos % arena->commit_size;

    if (target_commit_pos >= arena->commit_pos) { return; }

    u64 decommit_size = arena->commit_pos - target_commit_pos;
    u8 *decommit_start = (u8*)arena + target_commit_pos;

    if (plat_mem_decommit(decommit_start, decommit_size)) {
        arena->commit_pos = target_commit_pos;

        if (arena->mark_pos > arena->commit_pos) {
            arena->mark_pos = 0;
        }
    }
}

void *smrt_arena_push(smrt_arena_t *arena, u64 alloc_amount, b32 zero_out) {
    u64 pos_aligned = ALIGN_UP_POW2(arena->pos, ARENA_ALIGN);
    u64     new_pos = pos_aligned + alloc_amount;

    if (new_pos > arena->reserve_size) { return NULL; }

    if (new_pos > arena->commit_pos) {
        u64 new_commit_pos = new_pos;
        new_commit_pos += arena->commit_size - 1;
        new_commit_pos -= new_commit_pos % arena->commit_size;
        new_commit_pos =  MIN(new_commit_pos, arena->reserve_size);

        u8 *mem = (u8*)arena + arena->commit_pos;
        u64 commit_size = new_commit_pos - arena->commit_pos;

        if (!plat_mem_commit(mem, commit_size)) {
            return NULL;
        }

        arena->commit_pos = new_commit_pos;
    }

    arena->pos = new_pos;

    u8 *out = (u8*)arena + pos_aligned;

    if (zero_out) {
        memset(out, 0, alloc_amount);
    }

    return out;
}

void *smrt_arena_mark_push(smrt_arena_t *arena, u64 alloc_amount, b32 zero_out) {
    u64 orig_pos = arena->pos;

    void *mem = smrt_arena_push(arena, alloc_amount, zero_out);

    if (mem) {
        arena->mark_pos = orig_pos;
    }

    return mem;
}

void smrt_arena_pop(smrt_arena_t *arena, u64 pop_amount) {
    pop_amount = MIN(pop_amount, arena->pos - SMRT_ARENA_BASE_POS);
    arena->pos -= pop_amount;

    smrt_arena__shrink_commit(arena);
}

void smrt_arena_pop_to(smrt_arena_t *arena, u64 pos) {
    u64 size = pos < arena->pos ? arena->pos - pos : 0;
    smrt_arena_pop(arena, size);
}

b32 smrt_arena_pop_to_mark(smrt_arena_t *arena) {
    if (arena->mark_pos == 0) {
        return false;
    }

    smrt_arena_pop_to(arena, arena->mark_pos);

    return true;
}

void smrt_arena_clear(smrt_arena_t *arena, b32 zero_out) {
    arena->pos = SMRT_ARENA_BASE_POS;

    if (zero_out) {
        memset((u8*)arena + SMRT_ARENA_BASE_POS, 0, arena->commit_size - SMRT_ARENA_BASE_POS);
    }

    smrt_arena__shrink_commit(arena);
}

void smrt_arena_mark(smrt_arena_t *arena) {
    arena->mark_pos = arena->pos;
}

void smrt_arena_destroy(smrt_arena_t *arena) {
    plat_mem_release(arena, arena->reserve_size);
}

smrta_temp_t smrta_temp_start(smrt_arena_t *arena) {
    return (smrta_temp_t){
            .arena=arena,
        .start_pos=arena->pos
    };
}

void smrta_temp_end(smrta_temp_t temp) {
    smrt_arena_pop_to(temp.arena, temp.start_pos);
}

smrta_temp_t smrta_scratch_start(smrt_arena_t **conflicts, u32 num_conflicts) {
    i32 candidate_idx = -1;

    for (i32 i = 0; i < SCRATCH_POOL_SIZE; i++) {
        b32 conflict_found = false;

        for (u32 j = 0; j < num_conflicts; j++) {
            if (_scratch_pool[i] == conflicts[j]) {
                conflict_found = true;
            }
        }

        if (!conflict_found) {
            candidate_idx = i;
            break;
        }
    }

    if (candidate_idx != -1) {
        smrt_arena_t **selected = &_scratch_pool[candidate_idx];

        if (!*selected) {
            *selected = smrt_arena_create(SMRTA_SCRATCH_RESERVE_SIZE, plat_get_pagesize(), true);
        }

        return smrta_temp_start(*selected);
    }

    return (smrta_temp_t){ 0 };
}

void smrta_scratch_end(smrta_temp_t scratch) {
    smrta_temp_end(scratch);
}

#if defined(_WIN32)

#include <windows.h>

u32 plat_get_pagesize(void) {
    SYSTEM_INFO sysinfo = { 0 };
    GetSystemInfo(&sysinfo);

    return sysinfo.dwPageSize;
}

void* plat_mem_reserve(u64 size) {
    return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE);
}

b32 plat_mem_commit(void* ptr, u64 size) {
    void* ret = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
    return ret != NULL;
}

b32 plat_mem_decommit(void* ptr, u64 size) {
    return VirtualFree(ptr, size, MEM_DECOMMIT);
}

b32 plat_mem_release(void* ptr, u64 size) {
    return VirtualFree(ptr, size, MEM_RELEASE);
}


#elif defined(__linux__)

#include <unistd.h>
#include <sys/mman.h>

u32 plat_get_pagesize(void) {
    return (u32)sysconf(_SC_PAGESIZE);
}

void* plat_mem_reserve(u64 size) {
    void* out = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (out == MAP_FAILED) {
        return NULL;
    }
    return out;
}

b32 plat_mem_commit(void* ptr, u64 size) {
    i32 ret = mprotect(ptr, size, PROT_READ | PROT_WRITE);
    return ret == 0;
}

b32 plat_mem_decommit(void* ptr, u64 size) {
    i32 ret = mprotect(ptr, size, PROT_NONE);
    if (ret != 0) return false;
    ret = madvise(ptr, size, MADV_DONTNEED);
    return ret == 0;
}

b32 plat_mem_release(void* ptr, u64 size) {
    i32 ret = munmap(ptr, size);
    return ret == 0;
}

#endif
