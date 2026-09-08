#include "binary.h"

strng_t *strng_u64_bin(smrt_arena_t *arena, u64 n) {
    strng_t *s = strng_new(arena, 64);
    s->len = 64;
    for (u8 i = 0; i < 64; i++) {
        b8 set = (n & (1ull << i)) != 0;
        ((u8*)s)[STRNG_BASE_POS + (63 - i)] = set ? '1' : '0';
    }

    return s;
}

