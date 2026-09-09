#define _POSIX_C_SOURCE 200809L
#include "courier.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

static const char *priority_str(Priority p) {
    switch (p) {
        case EXPRESS:  return "EXPRESS";
        case STANDARD: return "STANDARD";
        case ECONOMY:  return "ECONOMY";
        default:       return "UNKNOWN";
    }
}

void *courier_thread(void *arg) {
    CourierArgs *a = (CourierArgs *)arg;

    while (1) {
        /* ---- Acquire queue mutex ---- */
        pthread_mutex_lock(a->queue_mutex);

        /* If queue empty and no shutdown yet: print WAITING then block */
        if (pq_size(a->queue) == 0 && !(*a->shutdown)) {
            pthread_mutex_lock(a->log_mutex);
            printf("[COURIER-%d] WAITING\n", a->courier_id);
            fflush(stdout);
            pthread_mutex_unlock(a->log_mutex);
        }

        /* Spurious wakeup guard: keep waiting while queue empty & not shutdown */
        while (pq_size(a->queue) == 0 && !(*a->shutdown)) {
            pthread_cond_wait(a->queue_cond, a->queue_mutex);
        }

        /* Woke up: shutdown with empty queue -> exit */
        if (*a->shutdown && pq_size(a->queue) == 0) {
            pthread_mutex_unlock(a->queue_mutex);
            break;
        }

        /* Pop highest-priority order */
        Order order;
        pq_pop(a->queue, &order);
        pthread_mutex_unlock(a->queue_mutex);

        /* ---- Deliver ---- */
        pthread_mutex_lock(a->log_mutex);
        printf("[COURIER-%d] DELIVERY_START id=%d recipient=%s priority=%s\n",
               a->courier_id, order.id, order.name, priority_str(order.priority));
        fflush(stdout);
        pthread_mutex_unlock(a->log_mutex);

        /* Simulate delivery: 1 unit = 500 ms */
        {
            long total_ns = (long)order.duration * 500L * 1000000L;
            struct timespec ts;
            ts.tv_sec  = total_ns / 1000000000L;
            ts.tv_nsec = total_ns % 1000000000L;
            nanosleep(&ts, NULL);
        }
        long duration_ms = (long)order.duration * 500L;

        pthread_mutex_lock(a->log_mutex);
        printf("[COURIER-%d] DELIVERY_COMPLETE id=%d recipient=%s duration=%ldms\n",
               a->courier_id, order.id, order.name, duration_ms);
        fflush(stdout);
        pthread_mutex_unlock(a->log_mutex);

        /* Update atomic global counters */
        atomic_fetch_add(a->completed_orders, 1);
        atomic_fetch_add(a->total_delivery_time, duration_ms);

        /* Update per-courier stats (only this thread writes) */
        a->stats->completed++;
        a->stats->total_time_ms += duration_ms;
    }

    pthread_mutex_lock(a->log_mutex);
    printf("[COURIER-%d] SHIFT_OVER\n", a->courier_id);
    fflush(stdout);
    pthread_mutex_unlock(a->log_mutex);

    return NULL;
}
