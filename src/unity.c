#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif /* ifndef _GNU_SOURCE */
#ifndef _DEFAULT_SOURCE
    #define _DEFAULT_SOURCE
#endif /* ifndef _DEFAULT_SOURCE */

// Config for global scratch arena allocations
#if 0
    #define SCRATCH_POOL_SIZE 2
    #define SMRTA_SCRATCH_RESERVE_SIZE MiB(64)
#endif /* if 0 */

// Arenas and mem management
#include "ez_arena.c"
#include "smrt_arena.c"

// Data structures
#include "string.c"
#include "vec2.c"
#include "slidingwindow.c"
#include "vec2sw.c"
#include "deque.c"

// DSP/Audio
#include "wav.c"
#include "dft.c"

// Misc
#include "log.c"
#include "threadpool.c"
#include "version.c"
