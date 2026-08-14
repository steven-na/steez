#pragma once

#include "common.h"
#include "deque.h"
#include "smrt_arena.h"

#include <bits/pthreadtypes.h>

typedef void (*tp_job_proc)(void *args);

typedef struct {
            u64  num_threads;
      pthread_t *    threads;
// Threads check this before waiting for a job
            b32   is_running;
// Write mutex for num_active/alive
pthread_mutex_t *  count_mtx;
// Signal to tp_wait that all jobs are complete
 pthread_cond_t *done_signal;
// Threads currently doing a job
            u64   num_active;
// Threads who are alive
            u64    num_alive;
// Job queue
     ts_deque_t         jobs;
} thread_pool_t;

// Thread pool will be allocated on the arena.
thread_pool_t *tp_create(smrt_arena_t *    arena ,
                                 u64    max_jobs ,
                                 u64 num_threads);

i32 tp_destroy(thread_pool_t *tp);

i32 tp_push_job(thread_pool_t *  tp ,
                  tp_job_proc   job ,
                         void *args);

// Wait for all threads to finish
void tp_wait(thread_pool_t *tp);
