#include "common.h"
#include "threadpool.h"
#include "smrt_arena.h"
#include "deque.h"

typedef struct {
    tp_job_proc proc;
    void *args;
} tp_job_t;

