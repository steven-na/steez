#pragma once

#include "common.h"
#include "smrt_arena.h"

#define STRNG_BASE_POS (sizeof(strng_t))

typedef struct {
    u64 alloc_size;
    u64 len;
} strng_t;

typedef struct {
       u64   start;
       u64     end;
       u64     max;
char const *string;
} strng_view_t;

strng_t * strng_new(smrt_arena_t *arena, u64 size);
strng_t *strng_from(smrt_arena_t *arena, char const *c);
strng_t * strng_dup(smrt_arena_t *arena, strng_t const *src);
   char * strng_str(smrt_arena_t *arena, strng_t const *string);
    i32   strng_set(strng_t *string, char const *c);
// Append to strng, returns -1 if no space, new len otherwise
    i32   strng_app(strng_t *dest, strng_t const *source);
// Append char* to strng, returns -1 if no space, new len otherwise
    i32 strng_app_c(strng_t *dest, char const *source);
// Append sv to strng, returns -1 if no space, new len otherwise
    i32 strng_app_v(strng_t *dest, strng_view_t const *source);
   void strng_clear(strng_t *string);

#define STRNG_TO(s) (char *)((u8*)(s)+STRNG_BASE_POS)
#define STRNG_FMT(s) (i32)s->len, (char *)((u8*)(s)+STRNG_BASE_POS)

static inline strng_view_t sv_from(strng_t *string) {
    return (strng_view_t){
        .string=(char*)((u8*)string+STRNG_BASE_POS),
        .start=0,
        .end=string->len,
        .max=string->len,
    };
}
strng_view_t sv_from_chars(char const* c);

void  sv_trim_left(strng_view_t *sv);
void sv_trim_right(strng_view_t *sv);
void       sv_trim(strng_view_t *sv);
void      sv_reset(strng_view_t *sv);

#define SV_FMT(sv) (i32)((sv)->end - (sv)->start), (char *)((u8*)((sv)->string) + (sv)->start)

