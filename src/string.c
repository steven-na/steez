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

strng_t *strng_from(smrt_arena_t *arena, char const *c) {
    u64 size = strlen(c);

    strng_t *s = strng_new(arena, size);

    if (!s) {
        return NULL;
    }

    memcpy((u8*)s+STRNG_BASE_POS, c, size);
    s->len = size;

    return s;
}

strng_t *strng_dup(smrt_arena_t *arena, strng_t const *src) {
    u64 size = src->len + sizeof(strng_t);

    strng_t *string = smrt_arena_push(arena, size, true);

    if (!string) {
        return NULL;
    }

    string->alloc_size = src->len;

    memcpy((u8*)string+STRNG_BASE_POS, (u8*)src+STRNG_BASE_POS, src->len);
    string->len = src->len;

    return string;
}

char *strng_str(smrt_arena_t *arena, strng_t const *string) {
    char *s = SMRTA_ALLOC_ARRAY(arena, char, string->len+1);

    if (!s) {
        return NULL;
    } else if (string->len == 0) {
        return s;
    }

    memcpy(s, (u8*)string+STRNG_BASE_POS, string->len);

    return s;
}

b32 strng_set(strng_t *string, char const *c) {
    u64 size = strlen(c);

    if (string->alloc_size < size) {
        return false;
    }

    memcpy((u8*)string+STRNG_BASE_POS, c, size);
    string->len = size;

    return true;
}

void strng_clear(strng_t *string) {
    memset((u8*)string+STRNG_BASE_POS, 0, string->len);
    string->len = 0;
}
