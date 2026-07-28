#pragma once

#include "common.h"
#include "smrt_arena.h"

#define STRNG_BASE_POS (sizeof(strng_t))

typedef struct {
    u64 alloc_size;
    u64 len;
} strng_t;

strng_t  *strng_new(smrt_arena_t *arena, u64 size);
strng_t *strng_from(smrt_arena_t *arena, char *c);
    b32   strng_set(strng_t *string, char *c);
   void strng_clear(strng_t *string);

#define STRNG_FMT(s) (int)s->len, (char *)((u8*)s+STRNG_BASE_POS)
