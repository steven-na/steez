#include "../src/string.h"

#include <stddef.h>
#include <stdio.h>

#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <criterion/internal/test.h>
#include <string.h>

Test(string, create_string) {
    smrt_arena_t *arena = smrt_arena_create(KiB(4), KiB(4), false);
    strng_t *s = strng_new(arena, 50);

    cr_expect(s->alloc_size == 50);
    cr_expect(s->len == 0);

    smrt_arena_destroy(arena);
}

Test(string, from_cstring) {
    smrt_arena_t *arena = smrt_arena_create(KiB(4), KiB(4), false);
    strng_t *s = strng_from(arena, "Hello, world.");

    cr_expect(s->alloc_size == 13);
    cr_expect(s->len == 13);
    cr_expect(*((u8*)s+STRNG_BASE_POS) == 72);
    cr_expect(*((u8*)s+STRNG_BASE_POS+s->len-1) == 46);

    smrt_arena_destroy(arena);
}

Test(string, set_string) {
    smrt_arena_t *arena = smrt_arena_create(KiB(4), KiB(4), false);
    strng_t *s = strng_from(arena, "Hello, world.");

    strng_set(s, "Look ma!");

    cr_expect(s->alloc_size == 13);
    cr_expect(s->len == 8);
    cr_expect(*((u8*)s+STRNG_BASE_POS) == 76);
    cr_expect(*((u8*)s+STRNG_BASE_POS+s->len-1) == 33);

    smrt_arena_destroy(arena);
}

Test(string, format_and_copy_string) {
    smrt_arena_t *arena = smrt_arena_create(KiB(4), KiB(4), false);

    strng_t *s = strng_from(arena, "Hello, world.");
    char *buffer = SMRTA_ALLOC_ARRAY(arena, char, s->len+1);

    u64 copied = snprintf(buffer, s->len+1, "%.*s", STRNG_FMT(s));
    cr_expect(copied == 13);
    cr_expect(strcmp(buffer, "Hello, world.") == 0);

    smrt_arena_destroy(arena);
}
