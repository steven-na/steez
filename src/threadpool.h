#pragma once

#include "common.h"
#include "deque.h"
#include "smrt_arena.h"

#include <bits/pthreadtypes.h>
#include <semaphore.h>

typedef void (*tp_job_proc)(void *args);

typedef struct {
            u64  num_threads;
      pthread_t *    threads;

            b32   is_running;

pthread_mutex_t *  count_mtx;
 pthread_cond_t *done_signal;

            u64   num_active;
            u64    num_alive;

     ts_deque_t         jobs;
} thread_pool_t;

// Thread pool will be allocated on the arena.
thread_pool_t *tp_create(smrt_arena_t *    arena ,
                                 u64    max_jobs ,
                                 u64 num_threads);

// This function waits for threads to finish, frees relevant allocations, and then frees *tp.
i32 tp_destroy(thread_pool_t *tp);

i32 tp_push_job(thread_pool_t *  tp ,
                  tp_job_proc   job ,
                         void *args);

// Wait for all threads to finish
void tp_wait(thread_pool_t *tp);
