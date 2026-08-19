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

#define SV_TO(sv) (char *)((u8*)((sv).string) + (sv).start)
#define SV_FMT(sv) (i32)(sv_len(&(sv))), SV_TO((sv))

static inline strng_view_t sv_from(strng_t const *string) {
    return (strng_view_t){
        .string=(char*)((u8*)string+STRNG_BASE_POS),
        .start=0,
        .end=string->len,
        .max=string->len,
    };
}

static inline u64 sv_len(strng_view_t const *sv) {
    return (sv->start <= sv->end) ? (sv->end - sv->start + 1) : 0;
}

static inline strng_view_t  sv_dup(strng_view_t const *src) { return *src; }
static inline strng_view_t sv_orig(strng_view_t const *src) { return (strng_view_t){
                                                                .string=src->string,
                                                                .start=0,
                                                                .end=src->max,
                                                                .max=src->max }; }
static inline strng_view_t sv_subv(strng_view_t const *src,
                                u64 start_off, u64 max_len) {
    u64 src_len = sv_len(src);
    u64 new_start = src->start + MIN(start_off, src_len);
    u64 available_len = (src->start + src_len) - new_start;
    u64 actual_len = MIN(available_len, max_len);

    return (strng_view_t){
        .string = src->string,
        .start = new_start,
        .end = actual_len == 0 ? new_start - 1 : new_start + actual_len - 1,
        .max = src->max
    };
}
static inline strng_view_t  sv_drop_left(strng_view_t const *src, u64 n) {
    u64 src_len = sv_len(src);
    u64 m = MIN(src_len, n);
    return sv_subv(src, m, src_len - m);
}
static inline strng_view_t sv_drop_right(strng_view_t const *src, u64 n) {
    u64 src_len = sv_len(src);
    u64 m = MIN(src_len, n);
    return sv_subv(src, 0, src_len - m);
}

strng_view_t sv_from_chars(char const* c);

void sv_trimws_left (strng_view_t *sv);
void sv_trimws_right(strng_view_t *sv);
void sv_trimws      (strng_view_t *sv);

void sv_trim_pastc_left (strng_view_t *sv, char n);
void sv_trim_pastc_right(strng_view_t *sv, char n);

// Starts from start and sets end to the next char n.
// Does nothing if there is no next char n
void sv_trim_end_nextc(strng_view_t *sv, char n);
// Starts from end and sets start to the prev char n.
// Does nothing if there is no prev char n
void sv_trim_strt_prvc(strng_view_t *sv, char n);

// Sets length keeping start
void sv_set_len_left (strng_view_t *sv, u64 n);
// Sets length keeping end
void sv_set_len_right(strng_view_t *sv, u64 n);


static inline void  sv_pop_left (strng_view_t *sv) { if (sv->start <= sv->end) sv->start++; }
static inline void  sv_pop_right(strng_view_t *sv) { if (sv->end >= sv->start)   sv->end--; }
static inline void sv_popn_left (strng_view_t *sv, u64 n) { for (u64 i = 0; i < n; i++)  sv_pop_left(sv); }
static inline void sv_popn_right(strng_view_t *sv, u64 n) { for (u64 i = 0; i < n; i++) sv_pop_right(sv); }

static inline void sv_reset(strng_view_t *sv) {
    sv->start=0;
    sv->end = sv->max;
}
static inline void sv_reset_right(strng_view_t *sv) {   sv->end = sv->max; }
static inline void  sv_reset_left(strng_view_t *sv) { sv->start =       0; }

// Returns -1 if not found, otherwise distance from sv->start to beginning of needle
i32 sv_find_substr(strng_view_t const *sv, char const *needle);
// Returns -1 if not found, otherwise distance from sv->start
i32   sv_find_char(strng_view_t const *sv, char n);

b32   sv_starts_with(strng_view_t const *sv, char const *prefix);
b32     sv_ends_with(strng_view_t const *sv, char const *suffix);
static inline b32 sv_starts_with_c(strng_view_t const *sv, char c) { return
                                                            (sv_len(sv) > 0) && (*SV_TO(*sv) == c); }
static inline b32   sv_ends_with_c(strng_view_t const *sv, char c) { return
                                                            (sv_len(sv) > 0) && (*(sv->string + sv->end) == c); }

b32 sv_eq_case_insensitive(strng_view_t const *sv1, strng_view_t const *sv2);

strng_view_t   sv_split_once(strng_view_t *sv, char const *needle);
strng_view_t sv_split_once_c(strng_view_t *sv, char n);

// Returns chars read on succes, -1 if couldn't read
// Creates its own SV and skips whitespace.
// Fails if first char after trim isnt +/- or 0-9
i32 sv_to_i64(strng_view_t const *sv, i64 *value_o);
