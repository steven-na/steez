#include "common.h"
#include "smrt_arena.h"
#include <string.h>

#include "string.h"

strng_t *strng_new(smrt_arena_t *arena, u64 size) {
    strng_t *s = smrt_arena_push(arena, sizeof(strng_t) + size, true);

    if (!s) {
        return NULL;
    }

    s->alloc_size = size;
    return s;
}

strng_t *strng_from(smrt_arena_t *arena, char *c) {
    u64 size = strlen(c);

    return strng_new(arena, size);
}

    b32   strng_set(strng_t *string, char *c);
   void strng_clear(strng_t *string);
