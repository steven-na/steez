#include "common.h"
#include "smrt_arena.h"
#include "log.h"
#include "string.h"

#include <ctype.h>
#include <string.h>

strng_t *strng_new(smrt_arena_t *arena, u64 size) {
    strng_t *s = smrt_arena_push(arena, sizeof(strng_t) + size, true);

    if (!s) {
        log_error("Failed to allocate strng");
        return NULL;
    }

    s->alloc_size = size;
    return s;
}

strng_t *strng_from(smrt_arena_t *arena, char const *c) {
    u64 size = strlen(c);

    strng_t *s = strng_new(arena, size);

    if (!s) {
        log_error("Failed to allocate strng");
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
        log_error("Failed to allocate strng");
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
        log_error("Failed to allocate char*");
        return NULL;
    } else if (string->len == 0) {
        return s;
    }

    memcpy(s, (u8*)string+STRNG_BASE_POS, string->len);

    return s;
}

i32 strng_set(strng_t *string, char const *c) {
    u64 size = strlen(c);

    if (string->alloc_size < size) return -1;

    memcpy((u8*)string+STRNG_BASE_POS, c, size);
    string->len = size;

    return 0;
}

i32 strng_app(strng_t *dest, strng_t const *source) {
    u64 slen = source->len;
    u64 nlen = dest->len + slen;
    if (nlen > dest->alloc_size) return -1;

    char *dst = STRNG_TO(dest)+dest->len;
    char const *src = STRNG_TO(source);
    memcpy(dst, src, slen);

    dest->len = nlen;
    return nlen;
}

i32 strng_app_c(strng_t *dest, char const *source) {
    u64 slen = strlen(source);
    u64 nlen = dest->len + slen;
    if (nlen > dest->alloc_size) return -1;

    char *dst = STRNG_TO(dest)+dest->len;
    char const *src = source;
    memcpy(dst, src, slen);

    dest->len = nlen;
    return nlen;
}

i32 strng_app_v(strng_t *dest, strng_view_t const *source) {
    u64 slen = source->end - source->start;
    u64 nlen = dest->len + slen;
    if (nlen > dest->alloc_size) return -1;

    char *dst = STRNG_TO(dest)+dest->len;
    char const *src = source->string+source->start;
    memcpy(dst, src, slen);

    dest->len = nlen;
    return nlen;
}

void strng_clear(strng_t *string) {
    memset((u8*)string+STRNG_BASE_POS, 0, string->len);
    string->len = 0;
}

strng_view_t sv_from_chars(char const* c) { u64 l = strlen(c);
                                            return (strng_view_t){
                                                .start=0,
                                                .end=l,
                                                .max=l,
                                                .string=c, }; }

void  sv_trim_left(strng_view_t *sv) {
    while (sv->start <= sv->end && isspace(*(sv->string+sv->start))) sv->start++;
    if (sv->start > sv->end) sv->start = sv->end;
}
void sv_trim_right(strng_view_t *sv) {
    while (sv->end >= sv->start && isspace(*(sv->string+sv->end))) sv->end--;
    if (sv->end < sv->start) sv->end = sv->start;
}
void       sv_trim(strng_view_t *sv) { sv_trim_left(sv);
                                      sv_trim_right(sv); }

void sv_reset(strng_view_t *sv) { sv->end=sv->max; sv->start=0; }
