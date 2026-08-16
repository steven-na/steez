#include "common.h"
#include "threadpool.h"
#include "smrt_arena.h"
#include "deque.h"
#include "log.h"

#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <string.h>

typedef struct {
    tp_job_proc  proc;
           void *args;
} tp_job_t;

typedef struct {
          u64  id;
thread_pool_t *tp;
} worker_args;

void *worker_proc(void *args) {
    worker_args *w_args = (worker_args*)args;
    thread_pool_t *tp = w_args->tp;

    #ifndef NLOG_TRACE
        log_trace("Spinning up thread %lu", w_args->id);
    #endif /* ifndef NLOG_TRACE */

    // Signal to tp_create that threads are done spinning up
    pthread_mutex_lock(tp->count_mtx); {
        if (++(tp->num_alive) == tp->num_threads) {
            pthread_cond_broadcast(tp->done_signal);
        }
    } pthread_mutex_unlock(tp->count_mtx);

    for (;;) {
        tp_job_t *j = (tp_job_t*)ts_deque_pop(tp->jobs);

        pthread_mutex_lock(tp->count_mtx);
        tp->num_active++;
        pthread_mutex_unlock(tp->count_mtx);

        j->proc(j->args);

        pthread_mutex_lock(tp->count_mtx);
        // If this is the last active thread finishing, tell someone about it
        if (--(tp->num_active) == 0) {
            pthread_cond_broadcast(tp->done_signal);

        #ifndef NLOG_TRACE
            log_trace("[Thread %lu] Broadcasting complete", w_args->id);
        #endif /* ifndef NLOG_TRACE */
        }

        if (!tp->is_running) {
            pthread_mutex_unlock(tp->count_mtx);
            break;
        }
        pthread_mutex_unlock(tp->count_mtx);
    }

    #ifndef NLOG_TRACE
        log_trace("Spinning down thread %lu", w_args->id);
    #endif /* ifndef NLOG_TRACE */

    pthread_mutex_lock(tp->count_mtx); {
        if (--(tp->num_alive) == 0) {
            pthread_cond_broadcast(tp->done_signal);
        }
    } pthread_mutex_unlock(tp->count_mtx);

    return NULL;
}

thread_pool_t *tp_create(smrt_arena_t *arena, u64 max_jobs, u64 num_threads) {
    thread_pool_t *tp = smrt_arena_push(arena, sizeof(thread_pool_t), true);

    ez_deque_t *q = ez_deque_create(arena, sizeof(tp_job_t), max_jobs);
    ts_deque_t tsq = ts_deque_create(arena, q);

    pthread_t *threads = SMRTA_ALLOC_ARRAY(arena, pthread_t, num_threads);
    // I just leak this into the arena
    worker_args * args = SMRTA_ALLOC_ARRAY(arena, worker_args, num_threads);

    pthread_mutex_t *count_mtx = smrt_arena_push(arena, sizeof(pthread_mutex_t), true);
    pthread_mutex_init(count_mtx, NULL);
    pthread_cond_t *all_done = smrt_arena_push(arena, sizeof(pthread_cond_t), true);
    pthread_cond_init(all_done, NULL);

    *tp = (thread_pool_t){
        .num_threads=num_threads,
        .threads=threads,

        .jobs=tsq,
        .is_running=true,

        .count_mtx=count_mtx,
        .done_signal=all_done,
        .num_alive=0,
        .num_active=0,
    };

    pthread_mutex_lock(tp->count_mtx);
    for (u64 i = 0; i < num_threads; i++) {
        args[i] = (worker_args){
            .id = i,
            .tp = tp,
        };
        pthread_create(&threads[i], NULL, worker_proc, (void*)&args[i]);
    }

    while (tp->num_alive != num_threads) {
        pthread_cond_wait(tp->done_signal, tp->count_mtx);
    }
    pthread_mutex_unlock(tp->count_mtx);


    #ifndef NLOG_TRACE
        log_trace("Created threadpool; thread count %lu", num_threads);
    #endif /* ifndef NLOG_TRACE */

    return tp;
}

void no_op(void *nothing) { (void)nothing; }

i32 tp_destroy(thread_pool_t *tp) {
    #ifndef NLOG_TRACE
        log_trace("Destroying threadpool");
    #endif /* ifndef NLOG_TRACE */

    pthread_mutex_lock(tp->count_mtx);
    tp->is_running = false;

    // Hand out Kool Aid
    u64 pills = tp->num_alive;
    for (u64 i = 0; i < pills; i++) tp_push_job(tp, no_op, NULL);
    while (tp->num_alive != 0) {
        pthread_cond_wait(tp->done_signal, tp->count_mtx);
    }

    for (u64 t = 0; t < tp->num_threads; t++) {
        pthread_join(tp->threads[t], NULL);
    }

    pthread_mutex_unlock(tp->count_mtx);
    ts_deque_destroy(tp->jobs);
    pthread_mutex_destroy(tp->count_mtx);
    pthread_cond_destroy(tp->done_signal);
    #ifndef NLOG_TRACE
        log_trace("Destroyed threadpool");
    #endif /* ifndef NLOG_TRACE */
    return 0;
}

i32 tp_push_job(thread_pool_t *tp, tp_job_proc job, void *args) {
    tp_job_t j = {
        .proc = job,
        .args = args,
    };

    #ifndef NLOG_TRACE
        log_trace("Pushing job %p with arg %p onto threadpool.", job, args);
    #endif /* ifndef NLOG_TRACE */

    return ts_deque_enqueue(tp->jobs, &j);
}

void tp_wait(thread_pool_t *tp) {
    #ifndef NLOG_TRACE
        log_trace("Threadpool waiting for %lu jobs", tp->num_active + tp->jobs.queue->occupied);
    #endif /* ifndef NLOG_TRACE */

    pthread_mutex_lock(tp->count_mtx);
    while (tp->jobs.queue->occupied || tp->num_active) {
        pthread_cond_wait(tp->done_signal, tp->count_mtx);
    }
    pthread_mutex_unlock(tp->count_mtx);

    #ifndef NLOG_TRACE
        log_trace("Threadpool finished waiting");
    #endif /* ifndef NLOG_TRACE */
}
