#pragma once

#include "common.h"
#include "smrt_arena.h"

#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <semaphore.h>

#define EZ_DEQUE_BASE_POS (sizeof(ez_deque_t))

typedef struct {
     u64               size;
     u64           occupied;
     u64              front;
     u64               back;
     u64 element_size_bytes;
} ez_deque_t;

#define SMRTA_ALLOC_EZDEQUE(arena, T, n) (ez_deque_t*)ez_deque_create((arena), sizeof(T), (n))

ez_deque_t *ez_deque_create(smrt_arena_t *arena, u64 elem_size, u64 size);

// Returns false if queue can't hold elem.
       b32 ez_deque_enqueue(ez_deque_t *q, void *elem);
// Returns deque backed memory. Copy if needed.
      void *   ez_deque_pop(ez_deque_t *q);

// Thread-safe deque (MPMC)
// Does not own queue
typedef struct {
     ez_deque_t *     queue;
pthread_mutex_t *write_lock;
          sem_t * count_sem;
} ts_deque_t;

ts_deque_t   ts_deque_create(ez_deque_t * q);
      void  ts_deque_destroy(ts_deque_t tsq);

// Deal with re-enqueueing yourself
       b32  ts_deque_enqueue(ts_deque_t tsq ,
                                 void *elem);
      void *    ts_deque_pop(ts_deque_t tsq);
