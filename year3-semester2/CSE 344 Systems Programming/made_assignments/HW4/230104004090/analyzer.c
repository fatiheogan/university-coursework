#define _GNU_SOURCE
#include "analyzer.h"
#include "shm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <limits.h>

/* ---------- Per-source hit tracking ---------- */
#define MAX_SOURCES_TRACK 256

typedef struct {
    char source[MAX_SOURCE_LEN];
    long hits;
} source_hit_t;

/* ---------- Shared state inside Analyzer Process ---------- */
typedef struct {
    int              level_idx;
    int              num_workers;
    int              num_keywords;
    char             keywords[MAX_KEYWORDS][64];
    shm_handle_t    *shm;
    const config_t  *cfg;

    pthread_barrier_t barrier;
    pthread_mutex_t   local_mutex;  /* protects per_thread_scores + source table */

    /* Accumulator filled by TLS destructors */
    double per_thread_scores[MAX_WORKERS][MAX_KEYWORDS];
    int    thread_keyword_written[MAX_WORKERS]; /* which slot each thread used */

    /* Source hit table */
    source_hit_t sources[MAX_SOURCES_TRACK];
    int          num_sources;

    /* Worker thread IDs for reporting-thread selection */
    pid_t   worker_tids[MAX_WORKERS];
    int     worker_indices[MAX_WORKERS]; /* maps tid slot -> worker index */

    /* Total entries processed */
    long total_entries;
} analyzer_shared_t;

/* TLS value: per-keyword weighted score array */
typedef struct {
    double scores[MAX_KEYWORDS];
    int    worker_idx;
    analyzer_shared_t *shared;
} tls_val_t;

static pthread_key_t tls_key;  /* one key per Analyzer process */

/* ---- TLS destructor: flush scores to shared area ---- */
static void tls_destructor(void *val) {
    if (!val) return;
    tls_val_t *tv = (tls_val_t *)val;
    analyzer_shared_t *sh = tv->shared;

    pthread_mutex_lock(&sh->local_mutex);
    int idx = tv->worker_idx;
    for (int k = 0; k < sh->num_keywords; k++)
        sh->per_thread_scores[idx][k] += tv->scores[k];
    sh->thread_keyword_written[idx] = 1;
    pthread_mutex_unlock(&sh->local_mutex);

    free(val);
}

/* ---- Update source hit table (caller holds local_mutex) ---- */
static void update_source(analyzer_shared_t *sh, const char *source, long weight) {
    for (int i = 0; i < sh->num_sources; i++) {
        if (strcmp(sh->sources[i].source, source) == 0) {
            sh->sources[i].hits += weight;
            return;
        }
    }
    if (sh->num_sources < MAX_SOURCES_TRACK) {
        strncpy(sh->sources[sh->num_sources].source, source, MAX_SOURCE_LEN - 1);
        sh->sources[sh->num_sources].source[MAX_SOURCE_LEN-1] = '\0';
        sh->sources[sh->num_sources].hits = weight;
        sh->num_sources++;
    }
}

/* ---------- Worker thread arg ---------- */
typedef struct {
    int               worker_idx;
    analyzer_shared_t *shared;
} worker_arg_t;

/* ---------- Worker thread function ---------- */
static void *worker_thread_func(void *varg) {
    worker_arg_t      *arg = (worker_arg_t *)varg;
    analyzer_shared_t *sh  = arg->shared;
    int                widx = arg->worker_idx;
    int                lvl  = sh->level_idx;
    int                wt   = level_weight(lvl);
    region_b_t        *rb   = sh->shm->region_b[lvl];
    (void)sh->cfg->capacity_b; /* capacity used via rb->capacity */

    /* Record TID */
    pid_t my_tid = (pid_t)syscall(SYS_gettid);
    pthread_mutex_lock(&sh->local_mutex);
    sh->worker_tids[widx]    = my_tid;
    sh->worker_indices[widx] = widx;
    pthread_mutex_unlock(&sh->local_mutex);

    printf("[PID:%d][TID:%d] Worker %d started.\n",
           (int)getpid(), (int)my_tid, widx);
    fflush(stdout);

    /* Allocate TLS value */
    tls_val_t *tv = calloc(1, sizeof(tls_val_t));
    if (!tv) { perror("calloc tls_val_t"); _exit(1); }
    tv->worker_idx = widx;
    tv->shared     = sh;
    pthread_setspecific(tls_key, tv);

    long entries_done = 0;
    double weighted_score = 0.0;

    while (1) {
        /* Pop from Region B */
        pthread_mutex_lock(&rb->level_mutex);
        while (rb->count == 0 && !rb->eof_posted)
            pthread_cond_wait(&rb->not_empty_b, &rb->level_mutex);

        if (rb->count == 0 && rb->eof_posted) {
            pthread_mutex_unlock(&rb->level_mutex);
            break;
        }

        log_entry_t entry;
        region_b_pop_locked(rb, &entry);
        pthread_cond_signal(&rb->not_full_b);
        pthread_mutex_unlock(&rb->level_mutex);

        if (entry.is_eof) break;  /* extra safety */

        entries_done++;

        /* Count keyword occurrences (overlapping) */
        long total_hits_this_entry = 0;
        for (int k = 0; k < sh->num_keywords; k++) {
            long cnt = count_keyword_occurrences(entry.message, sh->keywords[k]);
            double score = (double)(cnt * wt);
            tv->scores[k] += score;
            weighted_score += score;
            total_hits_this_entry += cnt;
        }

        /* Update source table */
        if (total_hits_this_entry > 0) {
            pthread_mutex_lock(&sh->local_mutex);
            update_source(sh, entry.source, total_hits_this_entry);
            pthread_mutex_unlock(&sh->local_mutex);
        }
    }

    pthread_mutex_lock(&sh->local_mutex);
    sh->total_entries += entries_done;
    pthread_mutex_unlock(&sh->local_mutex);

    printf("[PID:%d][TID:%d] Worker %d done. Entries: %ld, Weighted score: %.1f\n",
           (int)getpid(), (int)my_tid, widx, entries_done, weighted_score);
    fflush(stdout);

    /* Barrier: wait for all workers to finish consuming */
    pthread_barrier_wait(&sh->barrier);

    /* After barrier: TLS destructor will run at thread exit.
     * Only the reporting thread writes to Region C.
     * Find lowest TID among all workers. */

    /* Find minimum TID */
    pthread_mutex_lock(&sh->local_mutex);
    pid_t min_tid = sh->worker_tids[0];
    for (int i = 1; i < sh->num_workers; i++) {
        if (sh->worker_tids[i] > 0 && sh->worker_tids[i] < min_tid)
            min_tid = sh->worker_tids[i];
    }
    pthread_mutex_unlock(&sh->local_mutex);

    int is_reporter = (my_tid == min_tid);

    if (is_reporter) {
        printf("[PID:%d][TID:%d] ** Reporting thread (lowest TID). Level: %s **\n",
               (int)getpid(), (int)my_tid, level_str(lvl));
        fflush(stdout);
    }

    /* Thread exits here; destructor runs, flushing TLS scores */
    /* Reporting thread must wait for all destructors before writing Region C */
    /* We handle this below — after pthread_join in the process main */

    /* Pass reporter flag out via arg */
    arg->worker_idx = is_reporter ? -(widx + 1) : widx; /* encode reporter */

    return NULL;
}

/* ---------- Analyzer process main ---------- */
void analyzer_process_main(int level_idx, shm_handle_t *shm, const config_t *cfg) {
    printf("[PID:%d] Analyzer %s started. Workers: %d\n",
           (int)getpid(), level_str(level_idx), cfg->num_worker_threads);
    fflush(stdout);

    /* Create TLS key with destructor */
    if (pthread_key_create(&tls_key, tls_destructor) != 0) {
        perror("pthread_key_create");
        _exit(1);
    }

    /* Analyzer shared state */
    analyzer_shared_t sh;
    memset(&sh, 0, sizeof(sh));
    sh.level_idx   = level_idx;
    sh.num_workers = cfg->num_worker_threads;
    sh.num_keywords = cfg->num_keywords;
    sh.shm         = shm;
    sh.cfg         = cfg;
    for (int k = 0; k < cfg->num_keywords; k++)
        strncpy(sh.keywords[k], cfg->keywords[k], 63);

    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutex_init(&sh.local_mutex, &mattr);
    pthread_mutexattr_destroy(&mattr);

    pthread_barrier_init(&sh.barrier, NULL, cfg->num_worker_threads);

    /* Launch worker threads */
    pthread_t     *wtids = calloc(cfg->num_worker_threads, sizeof(pthread_t));
    worker_arg_t  *wargs = calloc(cfg->num_worker_threads, sizeof(worker_arg_t));

    for (int i = 0; i < cfg->num_worker_threads; i++) {
        wargs[i].worker_idx = i;
        wargs[i].shared     = &sh;
        if (pthread_create(&wtids[i], NULL, worker_thread_func, &wargs[i]) != 0) {
            perror("pthread_create worker");
            _exit(1);
        }
    }

    /* Wait for all workers to finish (including TLS destructor at exit) */
    for (int i = 0; i < cfg->num_worker_threads; i++) {
        pthread_join(wtids[i], NULL);
    }

    /* At this point all TLS destructors have run, per_thread_scores are filled */

    /* Compute total weighted score and per-keyword totals */
    double total_weighted = 0.0;
    double per_kw[MAX_KEYWORDS] = {0};
    double per_thread[MAX_WORKERS] = {0};

    for (int i = 0; i < cfg->num_worker_threads; i++) {
        for (int k = 0; k < cfg->num_keywords; k++) {
            per_kw[k]     += sh.per_thread_scores[i][k];
            per_thread[i] += sh.per_thread_scores[i][k];
            total_weighted += sh.per_thread_scores[i][k];
        }
    }

    printf("[PID:%d][TID:%d] Total entries: %ld | Total weighted score: %.1f\n",
           (int)getpid(), (int)syscall(SYS_gettid),
           sh.total_entries, total_weighted);
    fflush(stdout);

    /* Find top-3 sources */
    /* Simple selection sort */
    for (int i = 0; i < sh.num_sources; i++) {
        for (int j = i + 1; j < sh.num_sources; j++) {
            if (sh.sources[j].hits > sh.sources[i].hits) {
                source_hit_t tmp = sh.sources[i];
                sh.sources[i]   = sh.sources[j];
                sh.sources[j]   = tmp;
            }
        }
    }

    /* Write to Region C */
    region_c_t   *rc  = shm->region_c;
    level_result_t *lr = &rc->results[level_idx];

    pthread_mutex_lock(&rc->result_mutex);

    lr->total_entries        = sh.total_entries;
    lr->total_weighted_score = total_weighted;
    strncpy(lr->level, level_str(level_idx), sizeof(lr->level) - 1);

    for (int k = 0; k < cfg->num_keywords; k++)
        lr->per_keyword_score[k] = per_kw[k];

    for (int i = 0; i < cfg->num_worker_threads; i++)
        lr->per_thread_score[i] = per_thread[i];

    for (int i = 0; i < 3 && i < sh.num_sources; i++) {
        strncpy(lr->top_source[i], sh.sources[i].source, MAX_SOURCE_LEN - 1);
        lr->top_source_hits[i] = sh.sources[i].hits;
    }

    lr->ready = 1;
    rc->results_ready |= (1 << level_idx);
    pthread_cond_broadcast(&rc->result_cond);

    pthread_mutex_unlock(&rc->result_mutex);

    /* Post semaphore for Aggregator */
    sem_post(&rc->level_sem[level_idx]);

    /* Cleanup */
    pthread_barrier_destroy(&sh.barrier);
    pthread_mutex_destroy(&sh.local_mutex);
    pthread_key_delete(tls_key);
    free(wtids);
    free(wargs);

    printf("[PID:%d] Analyzer %s exiting.\n", (int)getpid(), level_str(level_idx));
    fflush(stdout);
    _exit(0);
}
