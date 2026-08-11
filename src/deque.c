#include "deque.h"
#include "smrt_arena.h"

#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>

ez_deque_t *ez_deque_create(smrt_arena_t *arena, u64 elem_size, u64 size) {
    u64 total_size = EZ_DEQUE_BASE_POS + (elem_size * size);

    ez_deque_t *q = smrt_arena_push(arena, total_size, true);
    q->size = size;
    q->element_size_bytes = elem_size;

    return q;
}

// Returns false if queue can't hold elem.
b32 ez_deque_enqueue(ez_deque_t *q, void *elem) {
    if (q->occupied < q->size) {
        u64 insert_pos = EZ_DEQUE_BASE_POS + (q->back * q->element_size_bytes);
        memcpy((u8*)q+insert_pos, elem, q->element_size_bytes);
        q->back = (q->back + 1) % q->size;
        q->occupied++;
        return true;
    }
    return false;
}

void *ez_deque_pop(ez_deque_t *q) {
    if (q->occupied == 0) return NULL;
    u64 pop_pos = EZ_DEQUE_BASE_POS + (q->front * q->element_size_bytes);
    q->front = (q->front + 1) % q->size;
    q->occupied--;
    return (u8*)q + pop_pos;
}

ts_deque_t ts_deque_create(ez_deque_t *q) {
    pthread_mutex_t *m = malloc(sizeof(pthread_mutex_t));
    *m = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

    sem_t *s = malloc(sizeof(sem_t));
    sem_init(s, 0, 0);

    return (ts_deque_t){
        .queue = q,
        .write_lock = m,
        .count_sem = s,
    };
}

void ts_deque_destroy(ts_deque_t tsq) {
    sem_destroy(tsq.count_sem);
    free(tsq.count_sem);
    pthread_mutex_destroy(tsq.write_lock);
    free(tsq.write_lock);
}

b32 ts_deque_enqueue(ts_deque_t tsq, void *elem) {
    pthread_mutex_lock(tsq.write_lock);
    b32 result = ez_deque_enqueue(tsq.queue, elem);
    pthread_mutex_unlock(tsq.write_lock);
    if (result) sem_post(tsq.count_sem);
    return result;
}

void *ts_deque_pop(ts_deque_t tsq) {
    sem_wait(tsq.count_sem);
    pthread_mutex_lock(tsq.write_lock);
    void *result = ez_deque_pop(tsq.queue);
    pthread_mutex_unlock(tsq.write_lock);
    return result;
}
