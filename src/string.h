#pragma once

#include "common.h"
#include "smrt_arena.h"

#define STRNG_BASE_POS (sizeof(strng_t))

typedef struct {
    u64 alloc_size;
    u64 len;
} strng_t;

strng_t * strng_new(smrt_arena_t *arena, u64 size);
strng_t *strng_from(smrt_arena_t *arena, char const *c);
strng_t * strng_dup(smrt_arena_t *arena, strng_t const *src);
   char * strng_str(smrt_arena_t *arena, strng_t const *string);
    b32   strng_set(strng_t *string, char const *c);
   void strng_clear(strng_t *string);

#define STRNG_FMT(s) (i32)s->len, (char *)((u8*)s+STRNG_BASE_POS)
