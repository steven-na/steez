#pragma once

#include "common.h"
#include "log.h"
#include "string.h"

// Binary representation of u64
strng_t *strng_u64_bin(smrt_arena_t *arena, u64 n);

static inline strng_view_t uint_to_binsv_impl(smrt_arena_t *arena, u64 n, u8 bits) {
    strng_t *b = strng_u64_bin(arena, n);
    strng_view_t svb = sv_from(b);
    sv_set_len_right(&svb, bits);
    return svb;
}

// Converts n (u8-u64) to a strng_view_t representing the 0s and 1s
#define UINT_TO_BINSV(arena, n) uint_to_binsv_impl(arena, n, \
        _Generic(n, u8: 8, u16: 16, u32: 32, default: 64))

#define LOG_UINT_BINREP(label, n) \
    do { \
        smrta_temp_t _macro_t = QUICK_SCRATCH; \
        auto _macro_n = (n); \
        strng_view_t _macro_svb = UINT_TO_BINSV(_macro_t.arena, _macro_n); \
        log_debug(label": 0b%.*s", SV_FMT(_macro_svb)); \
        QUICK_SCRATCH_E(_macro_t); \
    } while (0)
