#include "deque.h"
#include "smrt_arena.h"
#include "log.h"

#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

ez_deque_t *ez_deque_create(smrt_arena_t *arena, u64 elem_size, u64 size) {
    u64 total_size = EZ_DEQUE_BASE_POS + (elem_size * size);

    ez_deque_t *q = smrt_arena_mark_push(arena, total_size, true);
    if (!q) {
        log_error("Failed to allocate deque");
        smrt_arena_pop_to_mark(arena);
        return NULL;
    }
    q->size = size;
    q->element_size_bytes = elem_size;

    #ifndef NLOG_TRACE
        log_trace("Created deque; elem size %lu, capacity %lu", elem_size, size);
    #endif /* ifndef NLOG_TRACE */
    return q;
}

// Returns -1 if queue can't hold element, otherwise 0.
i32 ez_deque_enqueue(ez_deque_t *q, void *elem) {
    if (q->occupied < q->size) {
        u64 insert_pos = EZ_DEQUE_BASE_POS + (q->back * q->element_size_bytes);
        memcpy((u8*)q+insert_pos, elem, q->element_size_bytes);
        q->back = (q->back + 1) % q->size;
        q->occupied++;
        return 0;
    }
    return -1;
}

void *ez_deque_pop(ez_deque_t *q) {
    if (q->occupied == 0) return NULL;
    u64 pop_pos = EZ_DEQUE_BASE_POS + (q->front * q->element_size_bytes);
    q->front = (q->front + 1) % q->size;
    q->occupied--;
    return (u8*)q + pop_pos;
}

// Sync constructs are allocated on arena
ts_deque_t ts_deque_create(smrt_arena_t *arena, ez_deque_t *q) {
    pthread_mutex_t *m = smrt_arena_mark_push(arena, sizeof(pthread_mutex_t), true);
    if (!m) {
        log_error("Failed to allocate TSQ mutex");
        smrt_arena_pop_to_mark(arena);
        return (ts_deque_t){0};
    }

    pthread_mutex_init(m, NULL);

    sem_t *s = smrt_arena_push(arena, sizeof(sem_t), true);
    if (!s) {
        log_error("Failed to allocate TSQ semaphore");
        smrt_arena_pop_to_mark(arena);
        return (ts_deque_t){0};
    }
    sem_init(s, 0, 0);

    #ifndef NLOG_TRACE
        log_trace("Created thread-safe Deque; queue %p", q);
    #endif /* ifndef NLOG_TRACE */

    return (ts_deque_t){
        .queue = q,
        .write_lock = m,
        .count_sem = s,
    };
}

void ts_deque_destroy(ts_deque_t tsq) {
    sem_destroy(tsq.count_sem);
    pthread_mutex_destroy(tsq.write_lock);

    #ifndef NLOG_TRACE
        log_trace("Destroyed thread-safe Deque");
    #endif /* ifndef NLOG_TRACE */
}

i32 ts_deque_enqueue(ts_deque_t tsq, void *elem) {
    pthread_mutex_lock(tsq.write_lock);
    i32 result = ez_deque_enqueue(tsq.queue, elem);
    pthread_mutex_unlock(tsq.write_lock);
    if (result == 0) sem_post(tsq.count_sem);
    return result;
}

void *ts_deque_pop(ts_deque_t tsq) {
    sem_wait(tsq.count_sem);
    pthread_mutex_lock(tsq.write_lock);
    void *result = ez_deque_pop(tsq.queue);
    pthread_mutex_unlock(tsq.write_lock);
    return result;
}
