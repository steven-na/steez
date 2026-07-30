#include "../src/string.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <criterion/internal/test.h>

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
    cr_expect(*((u8*)s+STRNG_BASE_POS) == 'H');
    cr_expect(*((u8*)s+STRNG_BASE_POS+s->len-1) == '.');

    smrt_arena_destroy(arena);
}

Test(string, to_cstring) {
    smrt_arena_t *arena = smrt_arena_create(KiB(4), KiB(4), false);
    strng_t *s = strng_from(arena, "Heavy is the head");

    char *c = strng_str(arena, s);

    cr_expect(strlen(c) == 17);
    cr_expect(strcmp(c, "Heavy is the head") == 0);

    smrt_arena_destroy(arena);
}

Test(string, set_string) {
    smrt_arena_t *arena = smrt_arena_create(KiB(4), KiB(4), false);
    strng_t *s = strng_from(arena, "Hello, world.");

    strng_set(s, "Look ma!");

    cr_expect(s->alloc_size == 13);
    cr_expect(s->len == 8);
    cr_expect(*((u8*)s+STRNG_BASE_POS) == 'L');
    cr_expect(*((u8*)s+STRNG_BASE_POS+s->len-1) == '!');

    smrt_arena_destroy(arena);
}

Test(string, string_duplicate) {
    smrt_arena_t *arena = smrt_arena_create(KiB(4), KiB(4), false);

    strng_t *orig = strng_new(arena, 50);
    strng_set(orig,  "that falls with the weight");


    strng_t *dup = strng_dup(arena, orig);

    cr_expect(dup->alloc_size == 26);
    cr_expect(dup->len == 26);
    cr_expect(*((u8*)dup+STRNG_BASE_POS) == 't');
    cr_expect(*((u8*)dup+STRNG_BASE_POS+dup->len-1) == 't');

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
