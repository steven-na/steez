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

    strng_t *s = strng_new(arena, size);

    if (!s) {
        return NULL;
    }

    memcpy((u8*)s+STRNG_BASE_POS, c, size);
    s->len = size;

    return s;
}

b32 strng_set(strng_t *string, char *c) {
    u64 size = strlen(c);

    if (string->alloc_size < size) {
        return false;
    }

    memcpy((u8*)string+STRNG_BASE_POS, c, size);
    string->len = size;

    return true;
}

void strng_clear(strng_t *string) {
    string->len = 0;
}
