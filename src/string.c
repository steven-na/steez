#include "common.h"
#include "smrt_arena.h"
#include "log.h"
#include "string.h"

#include <immintrin.h>
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
                                                .end=l-1,
                                                .max=l-1,
                                                .string=c, }; }

void sv_trimws_left(strng_view_t *sv) {
    while (sv->start <= sv->end && isspace((unsigned char)*(sv->string+sv->start))) sv->start++;
}

void sv_trimws_right(strng_view_t *sv) {
    while (sv->end >= sv->start && isspace((unsigned char)*(sv->string+sv->end))) sv->end--;
}

void sv_trimws(strng_view_t *sv) {
    sv_trimws_left(sv);
    sv_trimws_right(sv);
}

void sv_trim_pastc_left(strng_view_t *sv, char n) {
    while (sv->start <= sv->end) {
        if (*(sv->string+sv->start) == n) {
            sv->start++;
            return;
        }
        sv->start++;
    }
}

void sv_trim_pastc_right(strng_view_t *sv, char n) {
    while (sv->end >= sv->start) {
        if (*(sv->string+sv->end) == n) {
            sv->end--;
            return;
        }
        sv->end--;
    }
}

void sv_trim_end_nextc(strng_view_t *sv, char n) {
    if (sv->start > sv->end) return;

    i32 i = sv_find_char(sv, n);

    if (i > -1) {
        if (i == 0) {
            sv->end = sv->start - 1;
        } else {
            sv->end = sv->start + i - 1;
        }
    }
}

void sv_trim_strt_prvc(strng_view_t *sv, char n) {
    if (sv->start > sv->end) return;

    for (i64 i = (i64)sv->end; i >= (i64)sv->start; i--) {
        if (*(sv->string + i) == n) {
            sv->start = i + 1;
            return;
        }
    }
}

void sv_set_len_left(strng_view_t *sv, u64 n) {
    if (sv->start > sv->end) return;

    if (n == 0) {
        sv->end = sv->start - 1;
        return;
    }
    u64 target_end = sv->start + n - 1;
    if (target_end < (u64)sv->end) {
        sv->end = (u64)target_end;
    }
}

void sv_set_len_right(strng_view_t *sv, u64 n) {
    if (sv->start > sv->end) return;

    if (n == 0) {
        sv->start = sv->end + 1;
        return;
    }

    if (n >= sv_len(sv)) return;

    sv->start = sv->end - n + 1;
}

i32 sv_find_substr(strng_view_t const *sv, char const *_needle) {
    u64 nlen = strlen(_needle);
    u64 hlen = sv_len(sv);

    if ((nlen > hlen) || !hlen) return -1;
    if (nlen == hlen && memcmp(sv->string+sv->start, _needle, hlen) == 0) return 0;

    char const *restrict haystack = sv->string+sv->start;
    char const *restrict   needle = _needle;

    __m256i first_vec = _mm256_set1_epi8(*needle);

    u64 end_idx = hlen - nlen + 1;
    u64 i = 0;

    for (; i + 32 <= end_idx; i += 32) {
        __m256i hay_vec = _mm256_loadu_si256((const __m256i*)(haystack+i));
        __m256i cmp = _mm256_cmpeq_epi8(hay_vec, first_vec);
        u32 mask = _mm256_movemask_epi8(cmp);

        while (mask != 0) {
            i32 offset = __builtin_ctz(mask);
            if (strncmp(haystack+i+offset, needle, nlen) == 0) return i+offset;
            mask &= (mask - 1);
        }
    }

    for (; i < end_idx; i++) { if (strncmp(haystack + i, needle, nlen) == 0) return i; }

    return -1;
}

i32 sv_find_char(strng_view_t const *sv, char n) {
    u64 hlen = sv_len(sv);

    char const *restrict haystack = SV_TO(*sv);

    if (hlen == 0) return -1;
    if (hlen == 1 && *haystack == n) return 0;


    __m256i needle_vec = _mm256_set1_epi8(n);
    u64 i = 0;

    for (; i + 32 <= hlen; i += 32) {
        __m256i hay_vec = _mm256_loadu_si256((const __m256i*)(haystack+i));
        __m256i cmp = _mm256_cmpeq_epi8(hay_vec, needle_vec);
        u32 mask = _mm256_movemask_epi8(cmp);

        if (mask) {
            i32 offset = __builtin_ctz(mask);
            return (i32)(i+offset);
        }
    }

    for (; i < hlen; i++) { if (haystack[i] == n) return i; }

    return -1;
}

b32 sv_starts_with(strng_view_t const *sv, char const *prefix) {
    u64 plen = strlen(prefix);
    u64 hlen = sv_len(sv);
    if (plen > hlen || hlen == 0) return false;
    return memcmp(SV_TO(*sv), prefix, plen) == 0;
}

b32 sv_ends_with(strng_view_t const *sv, char const *suffix) {
    u64 slen = strlen(suffix);
    u64 hlen = sv_len(sv);
    if (slen > hlen || hlen == 0) return false;
    return memcmp(SV_TO(*sv)+(hlen-slen), suffix, slen) == 0;
}

b32 sv_eq_case_insensitive(strng_view_t const *sv1, strng_view_t const *sv2) {
    u64 len1 = sv_len(sv1);
    u64 len2 = sv_len(sv2);
    if (len1 != len2) return false;

    char const *restrict c1 = SV_TO(*sv1);
    char const *restrict c2 = SV_TO(*sv2);

    // This subtracts 'A' to -128 and underflows <'A'
    __m256i const subv = _mm256_set1_epi8((char)193);
    // This is what 'Z' goes to when it is subbed by ^
    __m256i const cmpv = _mm256_set1_epi8(-103);
    // The difference between 'A' and 'a' is OR 0x20
    __m256i const  orv = _mm256_set1_epi8(0x20);

    u64 i = 0;
    for (; i + 32 <= len1; i += 32) {
        __m256i v1 = _mm256_loadu_si256((const __m256i*)(c1+i));
        __m256i v2 = _mm256_loadu_si256((const __m256i*)(c2+i));

        __m256i U1 = _mm256_sub_epi8(v1, subv);
        __m256i U2 = _mm256_sub_epi8(v2, subv);

        U1 = _mm256_cmpgt_epi8(U1, cmpv);
        U2 = _mm256_cmpgt_epi8(U2, cmpv);

        U1 = _mm256_andnot_si256(U1, orv);
        U2 = _mm256_andnot_si256(U2, orv);

        v1 = _mm256_or_si256(v1, U1);
        v2 = _mm256_or_si256(v2, U2);

        __m256i cmp = _mm256_cmpeq_epi64(v1, v2);
        u32 mask = _mm256_movemask_epi8(cmp);

        if (mask != 0xFFFFFFFF) return false;
    }

    for (; i < len1; i++) { if (tolower(c1[i]) != tolower(c2[i])) return false; }

    return true;
}

strng_view_t sv_split_once(strng_view_t *sv, char const *needle) {
    u64 nlen = strlen(needle);
    u64 hlen = sv_len(sv);
    if (hlen == 0) return *sv;

    i32 i = sv_find_substr(sv, needle);
    if (nlen == 0 || i == -1) {
        strng_view_t out = *sv;
        sv->start = sv->end+1;
        return out;
    }

    strng_view_t out = *sv;
    if (i == 0) {
        out.end = out.start - 1;
    } else {
        out.end = out.start + i - 1;
    }
    sv->start += (i + nlen);
    return out;
}

strng_view_t sv_split_once_c(strng_view_t *sv, char n) {
    u64 hlen = sv_len(sv);
    if (hlen == 0) return *sv;

    i32 i = sv_find_char(sv, n);
    if (i == -1) {
        strng_view_t out = *sv;
        sv->start = sv->end+1;
        return out;
    }

    strng_view_t out = *sv;
    if (i == 0) {
        out.end = out.start - 1;
    } else {
        out.end = out.start + i - 1;
    }
    sv->start += (i + 1);
    return out;
}


i32 sv_to_i64(strng_view_t const *sv, i64 *value_o) {
    char const *origin_ptr = SV_TO(*sv);

    strng_view_t a = sv_dup(sv);
    sv_trimws(&a);

    char const *digit = SV_TO(a);
    char const *end_ptr = a.string + a.end;

    i64 mod = 1;
    if (*digit == '-') {
        mod = -1;
        digit++;
    } else if (*digit == '+') {
        digit++;
    }

    if (digit > end_ptr || !(*digit >= '0' && *digit <= '9')) return -1;
    i64 result = 0;
    while (digit <= end_ptr && *digit >= '0' && *digit <= '9') {
        i64 d = *digit - '0';

        if (mod == 1) {
            if (result > (INT64_MAX / 10) || (result == (INT64_MAX / 10) && d > (INT64_MAX % 10))) {
                return -1;
            }
            result = (result * 10) + d;
        } else {
            if (result < (INT64_MIN / 10) || (result == (INT64_MIN / 10) && -d < (INT64_MIN % 10))) {
                return -1;
            }
            result = (result * 10) - d;
        }
        digit++;
    }

    *value_o = result;
    return (i32)(digit - origin_ptr);
}
