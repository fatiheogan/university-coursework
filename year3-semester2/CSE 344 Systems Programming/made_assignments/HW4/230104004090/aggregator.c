#define _GNU_SOURCE
#include "aggregator.h"
#include "shm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>

/* Wait for all 4 Analyzer results using pthread_cond_timedwait on Region C */
static int wait_for_results(region_c_t *rc, int timeout_sec) {
    struct timespec deadline;
    clock_gettime(CLOCK_REALTIME, &deadline);
    deadline.tv_sec += timeout_sec;

    pthread_mutex_lock(&rc->result_mutex);
    while (rc->results_ready != 0xF) {
        int ret = pthread_cond_timedwait(&rc->result_cond, &rc->result_mutex, &deadline);
        if (ret == ETIMEDOUT) {
            for (int lvl = 0; lvl < NUM_LEVELS; lvl++) {
                if (!(rc->results_ready & (1 << lvl)))
                    fprintf(stderr, "[PID:%d] Aggregator: timeout waiting for level %s\n",
                            (int)getpid(), level_str(lvl));
            }
            pthread_mutex_unlock(&rc->result_mutex);
            return -1;
        }
        if (ret != 0) {
            pthread_mutex_unlock(&rc->result_mutex);
            return -1;
        }
    }
    pthread_mutex_unlock(&rc->result_mutex);

    /* Print received messages */
    for (int lvl = 0; lvl < NUM_LEVELS; lvl++) {
        printf("[PID:%d] %s result received.\n", (int)getpid(), level_str(lvl));
        fflush(stdout);
    }
    return 0;
}

/* Drain Region D and compute high-priority weighted score */
static double drain_region_d(shm_handle_t *shm, const config_t *cfg) {
    region_d_t *rd  = shm->region_d;
    double      total = 0.0;

    while (1) {
        pthread_mutex_lock(&rd->priority_mutex);

        struct timespec dl;
        clock_gettime(CLOCK_REALTIME, &dl);
        dl.tv_sec += cfg->timeout_sec;

        while (rd->count == 0 && !rd->dispatcher_done) {
            int rc2 = pthread_cond_timedwait(&rd->not_empty_d, &rd->priority_mutex, &dl);
            if (rc2 == ETIMEDOUT) break;
        }

        if (rd->count == 0) {
            pthread_mutex_unlock(&rd->priority_mutex);
            break;
        }

        log_entry_t entry;
        region_d_pop_locked(rd, &entry);
        pthread_cond_signal(&rd->not_full_d);
        pthread_mutex_unlock(&rd->priority_mutex);

        if (entry.is_eof) break;

        int wt = level_weight(entry.level);
        for (int k = 0; k < cfg->num_keywords; k++) {
            long cnt = count_keyword_occurrences(entry.message, cfg->keywords[k]);
            total += (double)(cnt * wt);
        }
    }
    return total;
}

/* Sort levels by total_weighted_score descending (insertion sort) */
static void sort_levels(level_result_t *sorted[NUM_LEVELS]) {
    for (int i = 1; i < NUM_LEVELS; i++) {
        level_result_t *key = sorted[i];
        int j = i - 1;
        while (j >= 0 && sorted[j]->total_weighted_score < key->total_weighted_score) {
            sorted[j+1] = sorted[j];
            j--;
        }
        sorted[j+1] = key;
    }
}

/* Write human-readable output file */
static int write_text_output(const config_t *cfg, region_c_t *rc,
                              double total_weighted, double high_priority_score) {
    FILE *f = fopen(cfg->output_file, "w");
    if (!f) {
        perror("fopen output_file");
        return -1;
    }

    /* Keyword list */
    fprintf(f, "KEYWORD_LIST: ");
    for (int k = 0; k < cfg->num_keywords; k++) {
        if (k > 0) fprintf(f, ",");
        fprintf(f, "%s", cfg->keywords[k]);
    }
    fprintf(f, "\n");

    fprintf(f, "FILES: %d\n", cfg->num_files);
    fprintf(f, "TOTAL_WEIGHTED_SCORE: %.1f\n", total_weighted);
    fprintf(f, "HIGH_PRIORITY_SCORE: %.1f\n", high_priority_score);
    fprintf(f, "# Levels sorted by total_weighted_score DESC\n");

    /* Header row */
    fprintf(f, "%-6s %8s %14s", "LEVEL", "ENTRIES", "WEIGHTED_SCORE");
    for (int k = 0; k < cfg->num_keywords; k++)
        fprintf(f, "  %10s", cfg->keywords[k]);
    fprintf(f, "\n");

    /* Sort */
    level_result_t *sorted[NUM_LEVELS];
    for (int i = 0; i < NUM_LEVELS; i++) sorted[i] = &rc->results[i];
    sort_levels(sorted);

    for (int i = 0; i < NUM_LEVELS; i++) {
        level_result_t *lr = sorted[i];
        fprintf(f, "%-6s %8ld %14.1f", lr->level, lr->total_entries,
                lr->total_weighted_score);
        for (int k = 0; k < cfg->num_keywords; k++)
            fprintf(f, "  %10.1f", lr->per_keyword_score[k]);
        fprintf(f, "\n");
    }

    /* Top-3 sources per level */
    fprintf(f, "# Top-3 sources per level\n");
    for (int i = 0; i < NUM_LEVELS; i++) {
        level_result_t *lr = sorted[i];
        fprintf(f, "%s", lr->level);
        for (int s = 0; s < 3; s++) {
            if (lr->top_source[s][0])
                fprintf(f, " %s:%ld", lr->top_source[s], lr->top_source_hits[s]);
        }
        fprintf(f, "\n");
    }

    /* Per-thread contributions */
    fprintf(f, "# Per-thread contributions (weighted score)\n");
    for (int i = 0; i < NUM_LEVELS; i++) {
        level_result_t *lr = sorted[i];
        fprintf(f, "%s", lr->level);
        for (int t = 0; t < cfg->num_worker_threads; t++)
            fprintf(f, " thread_%d:%.1f", t, lr->per_thread_score[t]);
        fprintf(f, "\n");
    }

    fclose(f);
    return 0;
}

/* Write binary checkpoint file (atomic rename) */
static int write_binary_output(const config_t *cfg, region_c_t *rc,
                                double total_weighted, double high_priority_score) {
    char tmp_path[300];
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", cfg->binary_file);

    FILE *f = fopen(tmp_path, "wb");
    if (!f) { perror("fopen binary tmp"); return -1; }

    /* Header */
    uint32_t magic    = BINARY_MAGIC;
    uint32_t version  = BINARY_VERSION;
    uint32_t num_lvls = NUM_LEVELS;
    uint32_t num_kw   = (uint32_t)cfg->num_keywords;

    size_t r;
    r = fwrite(&magic,               sizeof(uint32_t), 1, f); if (r != 1) goto write_err;
    r = fwrite(&version,             sizeof(uint32_t), 1, f); if (r != 1) goto write_err;
    r = fwrite(&num_lvls,            sizeof(uint32_t), 1, f); if (r != 1) goto write_err;
    r = fwrite(&num_kw,              sizeof(uint32_t), 1, f); if (r != 1) goto write_err;
    r = fwrite(&total_weighted,      sizeof(double),   1, f); if (r != 1) goto write_err;
    r = fwrite(&high_priority_score, sizeof(double),   1, f); if (r != 1) goto write_err;

    /* One fwrite per level_result_t */
    for (int i = 0; i < NUM_LEVELS; i++) {
        r = fwrite(&rc->results[i], sizeof(level_result_t), 1, f);
        if (r != 1) goto write_err;
    }

    fclose(f);

    /* Atomic rename */
    if (rename(tmp_path, cfg->binary_file) != 0) {
        perror("rename binary file");
        return -1;
    }
    return 0;

write_err:
    fprintf(stderr, "[PID:%d] Aggregator: partial write to binary file\n", (int)getpid());
    fclose(f);
    unlink(tmp_path);
    return -1;
}

/* Background thread arg for draining Region D */
typedef struct {
    shm_handle_t   *shm;
    const config_t *cfg;
    double          result;
} drain_arg_t;

static void *drain_thread_func(void *varg) {
    drain_arg_t *a = (drain_arg_t *)varg;
    a->result = drain_region_d(a->shm, a->cfg);
    return NULL;
}

/* ---------- Aggregator process main ---------- */
void aggregator_process_main(shm_handle_t *shm, const config_t *cfg) {
    printf("[PID:%d] Aggregator started. Waiting for %d levels...\n",
           (int)getpid(), NUM_LEVELS);
    fflush(stdout);

    region_c_t *rc = shm->region_c;

    /* Start Region D drain thread in background (runs concurrently with analyzer wait) */
    drain_arg_t darg = { shm, cfg, 0.0 };
    pthread_t drain_tid;
    pthread_create(&drain_tid, NULL, drain_thread_func, &darg);

    /* Wait for all Analyzer results via pthread_cond_timedwait on Region C */
    if (wait_for_results(rc, cfg->timeout_sec) != 0) {
        fprintf(stderr, "[PID:%d] Aggregator: not all results received, writing partial output.\n",
                (int)getpid());
    }

    /* Wait for drain thread to finish */
    pthread_join(drain_tid, NULL);
    double high_priority_score = darg.result;

    /* Store in shared memory so parent can read it */
    shm->region_d->high_priority_score = high_priority_score;

    /* Compute total weighted score */
    double total_weighted = 0.0;
    for (int i = 0; i < NUM_LEVELS; i++)
        total_weighted += rc->results[i].total_weighted_score;

    printf("[PID:%d] All results received. Writing output files...\n", (int)getpid());
    fflush(stdout);

    if (write_text_output(cfg, rc, total_weighted, high_priority_score) != 0)
        fprintf(stderr, "Failed to write text output\n");

    if (write_binary_output(cfg, rc, total_weighted, high_priority_score) != 0)
        fprintf(stderr, "Failed to write binary output\n");

    printf("[PID:%d] Output files written: %s, %s\n",
           (int)getpid(), cfg->output_file, cfg->binary_file);
    fflush(stdout);

    printf("[PID:%d] Aggregator exiting.\n", (int)getpid());
    fflush(stdout);
    _exit(0);
}
