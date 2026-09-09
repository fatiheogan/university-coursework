#ifndef COURIER_H
#define COURIER_H

#include <stdatomic.h>
#include <pthread.h>
#include "queue.h"

/* Per-courier statistics (non-atomic, protected by courier's own data since
   only that courier writes; read only after pthread_join) */
typedef struct {
    int completed;
    long total_time_ms;
} CourierStats;

/* Shared state passed to every courier thread */
typedef struct {
    /* Queue */
    PriorityQueue  *queue;
    pthread_mutex_t *queue_mutex;
    pthread_cond_t  *queue_cond;

    /* Shutdown flag – set to 1 on SIGINT */
    volatile int   *shutdown;

    /* Global atomic counters */
    _Atomic int    *completed_orders;
    _Atomic int    *cancelled_orders;
    _Atomic long   *total_delivery_time;

    /* Log mutex */
    pthread_mutex_t *log_mutex;

    /* This courier's id (1-based) and stats slot */
    int             courier_id;
    CourierStats   *stats;
} CourierArgs;

void *courier_thread(void *arg);

#endif /* COURIER_H */
