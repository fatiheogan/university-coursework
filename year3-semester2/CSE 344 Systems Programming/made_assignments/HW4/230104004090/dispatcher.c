#define _GNU_SOURCE
#include "dispatcher.h"
#include "shm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

/* Load priority source list from file */
static char priority_sources[1024][MAX_SOURCE_LEN];
static int  num_priority_sources = 0;

static void load_priority_sources(const char *filter_file) {
    FILE *f = fopen(filter_file, "r");
    if (!f) {
        fprintf(stderr, "[PID:%d] Dispatcher: cannot open filter file %s: %s\n",
                (int)getpid(), filter_file, strerror(errno));
        return;
    }
    char line[MAX_SOURCE_LEN + 4];
    while (fgets(line, sizeof(line), f) && num_priority_sources < 1024) {
        /* Strip newline */
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[--len] = '\0';
        if (len == 0) continue;
        strncpy(priority_sources[num_priority_sources], line, MAX_SOURCE_LEN - 1);
        priority_sources[num_priority_sources][MAX_SOURCE_LEN - 1] = '\0';
        num_priority_sources++;
    }
    fclose(f);
}

static int is_priority_source(const char *source) {
    for (int i = 0; i < num_priority_sources; i++) {
        if (strcmp(priority_sources[i], source) == 0) return 1;
    }
    return 0;
}

void dispatcher_process_main(shm_handle_t *shm, const config_t *cfg) {
    printf("[PID:%d] Dispatcher started.\n", (int)getpid());
    fflush(stdout);

    load_priority_sources(cfg->filter_file);

    region_a_t *ra = shm->region_a;
    region_d_t *rd = shm->region_d;
    int cap_d = cfg->capacity_d;

    /* Track how many EOF markers observed per level */
    int eof_observed[NUM_LEVELS] = {0};
    int eof_forwarded[NUM_LEVELS] = {0};
    int total_readers = cfg->num_files;

    while (1) {
        /* Check if all levels got all EOFs */
        int all_done = 1;
        for (int l = 0; l < NUM_LEVELS; l++) {
            if (!eof_forwarded[l]) { all_done = 0; break; }
        }
        if (all_done) break;

        /* Wait for an entry in Region A with timed wait */
        struct timespec deadline;
        clock_gettime(CLOCK_REALTIME, &deadline);
        deadline.tv_sec += cfg->timeout_sec;

        pthread_mutex_lock(&ra->input_mutex);
        while (ra->count == 0) {
            int rc = pthread_cond_timedwait(&ra->not_empty_a, &ra->input_mutex, &deadline);
            if (rc == ETIMEDOUT) {
                /* Check if all levels are done */
                int done = 1;
                for (int l = 0; l < NUM_LEVELS; l++) {
                    if (ra->eof_count_per_level[l] < total_readers) { done = 0; break; }
                }
                pthread_mutex_unlock(&ra->input_mutex);
                if (done) goto dispatcher_exit;
                /* Refresh deadline and retry */
                clock_gettime(CLOCK_REALTIME, &deadline);
                deadline.tv_sec += cfg->timeout_sec;
                pthread_mutex_lock(&ra->input_mutex);
                continue;
            }
            if (rc != 0) {
                pthread_mutex_unlock(&ra->input_mutex);
                goto dispatcher_exit;
            }
        }

        log_entry_t entry;
        region_a_pop_locked(ra, &entry);
        pthread_cond_signal(&ra->not_full_a);
        pthread_mutex_unlock(&ra->input_mutex);

        /* Handle EOF marker */
        if (entry.is_eof) {
            int lvl = entry.level;
            eof_observed[lvl]++;

            if (eof_observed[lvl] >= total_readers && !eof_forwarded[lvl]) {
                /* Forward EOF to Region B for this level */
                region_b_t *rb = shm->region_b[lvl];
                int cap_b = cfg->capacity_b;

                pthread_mutex_lock(&rb->level_mutex);
                while (rb->count >= cap_b)
                    pthread_cond_wait(&rb->not_full_b, &rb->level_mutex);
                log_entry_t eof_fwd;
                memset(&eof_fwd, 0, sizeof(eof_fwd));
                eof_fwd.is_eof = 1;
                eof_fwd.level  = lvl;
                region_b_push_locked(rb, &eof_fwd);
                rb->eof_posted = 1;
                pthread_cond_broadcast(&rb->not_empty_b);
                pthread_mutex_unlock(&rb->level_mutex);

                eof_forwarded[lvl] = 1;
            }
            continue;
        }

        /* Real entry: route to Region B for the appropriate level */
        int lvl    = entry.level;
        int is_pri = is_priority_source(entry.source);

        printf("[PID:%d] Routed entry to %s buffer. High-priority: %s (source: %s)\n",
               (int)getpid(), level_str(lvl),
               is_pri ? "YES" : "NO", entry.source);
        fflush(stdout);

        /* Push to Region B[lvl] */
        region_b_t *rb = shm->region_b[lvl];
        int cap_b = cfg->capacity_b;

        pthread_mutex_lock(&rb->level_mutex);
        while (rb->count >= cap_b)
            pthread_cond_wait(&rb->not_full_b, &rb->level_mutex);
        region_b_push_locked(rb, &entry);
        pthread_cond_signal(&rb->not_empty_b);
        pthread_mutex_unlock(&rb->level_mutex);

        /* Also push to Region D if high-priority */
        if (is_pri) {
            pthread_mutex_lock(&rd->priority_mutex);
            while (rd->count >= cap_d)
                pthread_cond_wait(&rd->not_full_d, &rd->priority_mutex);
            region_d_push_locked(rd, &entry);
            pthread_cond_signal(&rd->not_empty_d);
            pthread_mutex_unlock(&rd->priority_mutex);
        }
    }

dispatcher_exit:
    /* Signal Aggregator that Dispatcher is done with Region D */
    pthread_mutex_lock(&rd->priority_mutex);
    rd->dispatcher_done = 1;
    pthread_cond_broadcast(&rd->not_empty_d);
    pthread_mutex_unlock(&rd->priority_mutex);

    printf("[PID:%d] All EOF markers forwarded to Region B. Exiting.\n", (int)getpid());
    fflush(stdout);
    _exit(0);
}
