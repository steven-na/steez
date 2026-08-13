#pragma once

#include "common.h"
#include "deque.h"
#include "smrt_arena.h"

#include <bits/pthreadtypes.h>
#include <semaphore.h>

typedef void (*tp_job_proc)(void *args);

typedef struct {
    u64 num_threads;
    pthread_t *threads;

    sem_t threads_running;
    ts_deque_t jobs;
} thread_pool_t;

// Job queue will be allocated on the arena.
thread_pool_t tp_create(smrt_arena_t *      arena ,
                                  u64    max_jobs ,
                                  u64 num_threads);

i32 tp_destroy(thread_pool_t tp);

i32 tp_push_job(thread_pool_t     tp ,
                      tp_job_proc   job ,
                          void *args);

// Wait for all threads to finish
void tp_wait(thread_pool_t tp);
