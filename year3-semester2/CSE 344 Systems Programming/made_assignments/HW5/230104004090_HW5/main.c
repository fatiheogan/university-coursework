#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <unistd.h>
#include <errno.h>

#include "queue.h"
#include "courier.h"

/* ------------------------------------------------------------------ */
/* Shared global state                                                  */
/* ------------------------------------------------------------------ */
static PriorityQueue   g_queue;
static pthread_mutex_t g_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_queue_cond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t g_log_mutex   = PTHREAD_MUTEX_INITIALIZER;

static volatile int    g_shutdown = 0;

static _Atomic int     g_completed_orders   = 0;
static _Atomic int     g_cancelled_orders   = 0;
static _Atomic long    g_total_delivery_time = 0;

/* ------------------------------------------------------------------ */
/* Helpers                                                              */
/* ------------------------------------------------------------------ */
static void usage(const char *prog) {
    fprintf(stderr,
            "Usage: %s -n <num_couriers> -i <orders.txt> -s <stats.txt>\n",
            prog);
}

static Priority parse_priority(const char *s) {
    if (strcmp(s, "EXPRESS")  == 0) return EXPRESS;
    if (strcmp(s, "STANDARD") == 0) return STANDARD;
    if (strcmp(s, "ECONOMY")  == 0) return ECONOMY;
    return (Priority)0; /* invalid */
}

static const char *priority_str(Priority p) {
    switch (p) {
        case EXPRESS:  return "EXPRESS";
        case STANDARD: return "STANDARD";
        case ECONOMY:  return "ECONOMY";
        default:       return "UNKNOWN";
    }
}

/*
 * Parse orders file and push valid orders into the queue.
 * Returns number of orders loaded, or -1 on file open error.
 */
static int load_orders(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror(path);
        return -1;
    }

    char line[256];
    int  count = 0;

    while (fgets(line, sizeof(line), fp)) {
        /* Skip blank lines */
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0') continue;

        int  id;
        char name[MAX_NAME_LEN];
        char prio_str[16];
        int  duration;

        if (sscanf(line, "%d %32s %15s %d", &id, name, prio_str, &duration) != 4)
            continue;  /* malformed – skip silently */

        Priority p = parse_priority(prio_str);
        if (p == 0 || duration <= 0 || id <= 0)
            continue;  /* invalid values – skip silently */

        Order o;
        o.id       = id;
        o.priority = p;
        o.duration = duration;
        strncpy(o.name, name, MAX_NAME_LEN - 1);
        o.name[MAX_NAME_LEN - 1] = '\0';

        pq_push(&g_queue, o);

        pthread_mutex_lock(&g_log_mutex);
        printf("[CARGOGTU] ORDER_QUEUED id=%d recipient=%s priority=%s duration=%d\n",
               o.id, o.name, priority_str(o.priority), o.duration);
        fflush(stdout);
        pthread_mutex_unlock(&g_log_mutex);

        count++;
    }
    fclose(fp);
    return count;
}

/*
 * Write statistics file.
 * Called after all threads have joined.
 */
static void write_stats(const char *path, int num_couriers,
                         int total_orders,
                         CourierStats *stats) {
    FILE *fp = fopen(path, "w");
    if (!fp) {
        perror(path);
        return;
    }

    int  completed  = atomic_load(&g_completed_orders);
    int  cancelled  = atomic_load(&g_cancelled_orders);
    long total_time = atomic_load(&g_total_delivery_time);
    long avg        = (completed > 0) ? (total_time / completed) : 0;

    fprintf(fp, "SHIFT_SUMMARY\n");
    fprintf(fp, "Total orders : %d\n", total_orders);
    fprintf(fp, "Completed    : %d\n", completed);
    fprintf(fp, "Cancelled    : %d\n", cancelled);
    fprintf(fp, "Total time   : %ldms\n", total_time);
    fprintf(fp, "Avg per order: %ldms\n", avg);
    fprintf(fp, "COURIER_STATS\n");
    for (int i = 0; i < num_couriers; i++) {
        fprintf(fp, "Courier-%d completed=%d total_time=%ldms\n",
                i + 1, stats[i].completed, stats[i].total_time_ms);
    }
    fclose(fp);
}

/* ------------------------------------------------------------------ */
/* main                                                                 */
/* ------------------------------------------------------------------ */
int main(int argc, char *argv[]) {
    /* ---- Parse arguments ---- */
    int   num_couriers = 0;
    char *input_file   = NULL;
    char *stats_file   = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            num_couriers = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            input_file = argv[++i];
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            stats_file = argv[++i];
        }
    }

    if (num_couriers < 1 || !input_file || !stats_file) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    /* ---- Block SIGINT in all threads (sigwait approach) ---- */
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    if (pthread_sigmask(SIG_BLOCK, &mask, NULL) != 0) {
        perror("pthread_sigmask");
        return EXIT_FAILURE;
    }

    /* ---- Init queue and load orders ---- */
    pq_init(&g_queue);

    int total_orders = load_orders(input_file);
    if (total_orders < 0) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    /* ---- SHIFT_START ---- */
    pthread_mutex_lock(&g_log_mutex);
    printf("[CARGOGTU] SHIFT_START couriers=%d orders=%d\n",
           num_couriers, total_orders);
    fflush(stdout);
    pthread_mutex_unlock(&g_log_mutex);

    /* ---- Allocate per-courier stats and args ---- */
    CourierStats *stats = calloc((size_t)num_couriers, sizeof(CourierStats));
    CourierArgs  *args  = calloc((size_t)num_couriers, sizeof(CourierArgs));
    pthread_t    *tids  = calloc((size_t)num_couriers, sizeof(pthread_t));

    if (!stats || !args || !tids) {
        fprintf(stderr, "Out of memory\n");
        return EXIT_FAILURE;
    }

    /* ---- Spawn courier threads ---- */
    for (int i = 0; i < num_couriers; i++) {
        args[i].queue              = &g_queue;
        args[i].queue_mutex        = &g_queue_mutex;
        args[i].queue_cond         = &g_queue_cond;
        args[i].shutdown           = &g_shutdown;
        args[i].completed_orders   = &g_completed_orders;
        args[i].cancelled_orders   = &g_cancelled_orders;
        args[i].total_delivery_time= &g_total_delivery_time;
        args[i].log_mutex          = &g_log_mutex;
        args[i].courier_id         = i + 1;
        args[i].stats              = &stats[i];

        if (pthread_create(&tids[i], NULL, courier_thread, &args[i]) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    }

    /* ---- Wait for SIGINT or natural end ---- */
    /*
     * sigtimedwait polls for SIGINT every 100ms.
     * Between polls we check if the queue is empty AND all couriers
     * have finished (completed+cancelled == total_orders).
     * When that happens we do a clean shutdown broadcast.
     */
    struct timespec timeout;
    timeout.tv_sec  = 0;
    timeout.tv_nsec = 100000000L; /* 100 ms */

    while (1) {
        int ret = sigtimedwait(&mask, NULL, &timeout);
        if (ret == SIGINT) {
            /* ---- SIGINT received ---- */
            pthread_mutex_lock(&g_queue_mutex);
            g_shutdown = 1;

            /* Count and cancel pending orders */
            int pending = (int)pq_size(&g_queue);

            pthread_mutex_lock(&g_log_mutex);
            printf("[CARGOGTU] SIGINT_RECEIVED pending_orders=%d\n", pending);
            fflush(stdout);
            pthread_mutex_unlock(&g_log_mutex);

            /* Drain queue and cancel each order */
            Order o;
            while (pq_pop(&g_queue, &o)) {
                atomic_fetch_add(&g_cancelled_orders, 1);
                pthread_mutex_lock(&g_log_mutex);
                printf("[CARGOGTU] ORDER_CANCELLED id=%d recipient=%s priority=%s\n",
                       o.id, o.name, priority_str(o.priority));
                fflush(stdout);
                pthread_mutex_unlock(&g_log_mutex);
            }

            /* Wake all waiting couriers so they can exit */
            pthread_cond_broadcast(&g_queue_cond);
            pthread_mutex_unlock(&g_queue_mutex);
            break;
        }

        /* Timeout or EINTR – check if all work is naturally done */
        int done = atomic_load(&g_completed_orders) +
                   atomic_load(&g_cancelled_orders);
        if (done == total_orders) {
            /* All orders accounted for – set shutdown so WAITING couriers exit */
            pthread_mutex_lock(&g_queue_mutex);
            g_shutdown = 1;
            pthread_cond_broadcast(&g_queue_cond);
            pthread_mutex_unlock(&g_queue_mutex);
            break;
        }
    }

    /* ---- Join all courier threads ---- */
    for (int i = 0; i < num_couriers; i++) {
        pthread_join(tids[i], NULL);
    }

    /* ---- Final output ---- */
    int  completed  = atomic_load(&g_completed_orders);
    int  cancelled  = atomic_load(&g_cancelled_orders);
    long total_time = atomic_load(&g_total_delivery_time);

    pthread_mutex_lock(&g_log_mutex);
    printf("[CARGOGTU] SHIFT_END completed=%d cancelled=%d total_time=%ldms\n",
           completed, cancelled, total_time);
    fflush(stdout);
    pthread_mutex_unlock(&g_log_mutex);

    /* ---- Write stats file ---- */
    write_stats(stats_file, num_couriers, total_orders, stats);

    pthread_mutex_lock(&g_log_mutex);
    printf("[CARGOGTU] SHUTDOWN_COMPLETE\n");
    fflush(stdout);
    pthread_mutex_unlock(&g_log_mutex);

    /* ---- Cleanup ---- */
    pq_destroy(&g_queue);
    pthread_mutex_destroy(&g_queue_mutex);
    pthread_cond_destroy(&g_queue_cond);
    pthread_mutex_destroy(&g_log_mutex);
    free(stats);
    free(args);
    free(tids);

    return EXIT_SUCCESS;
}
