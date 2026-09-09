#define _GNU_SOURCE
#include "shm.h"
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/* ---------- Size helpers ---------- */

size_t region_a_total_size(int capacity) {
    return sizeof(region_a_t) + (size_t)(capacity - 1) * sizeof(log_entry_t);
}

size_t region_b_total_size(int capacity) {
    return sizeof(region_b_t) + (size_t)(capacity - 1) * sizeof(log_entry_t);
}

size_t region_c_total_size(void) {
    return sizeof(region_c_t);
}

size_t region_d_total_size(int capacity) {
    return sizeof(region_d_t) + (size_t)(capacity - 1) * sizeof(log_entry_t);
}

/* ---------- Helper: init process-shared mutex ---------- */
static int init_ps_mutex(pthread_mutex_t *m) {
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr) != 0) return -1;
    if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) != 0) {
        pthread_mutexattr_destroy(&attr);
        return -1;
    }
    int r = pthread_mutex_init(m, &attr);
    pthread_mutexattr_destroy(&attr);
    return r;
}

/* ---------- Helper: init process-shared condvar ---------- */
static int init_ps_cond(pthread_cond_t *c) {
    pthread_condattr_t attr;
    if (pthread_condattr_init(&attr) != 0) return -1;
    if (pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) != 0) {
        pthread_condattr_destroy(&attr);
        return -1;
    }
    int r = pthread_cond_init(c, &attr);
    pthread_condattr_destroy(&attr);
    return r;
}

/* ---------- mmap helper ---------- */
static void *do_mmap(size_t size) {
    void *p = mmap(NULL, size, PROT_READ | PROT_WRITE,
                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) return NULL;
    memset(p, 0, size);
    return p;
}

/* ---------- shm_init ---------- */
int shm_init(shm_handle_t *shm, const config_t *cfg) {
    memset(shm, 0, sizeof(*shm));

    /* --- Region A --- */
    shm->region_a_size = region_a_total_size(cfg->capacity_a);
    shm->region_a = do_mmap(shm->region_a_size);
    if (!shm->region_a) { perror("mmap region_a"); return -1; }

    region_a_t *ra = shm->region_a;
    ra->capacity     = cfg->capacity_a;
    ra->total_readers = cfg->num_files;
    if (init_ps_mutex(&ra->input_mutex) != 0) { perror("mutex region_a"); return -1; }
    if (init_ps_cond(&ra->not_full_a)   != 0) { perror("cond not_full_a"); return -1; }
    if (init_ps_cond(&ra->not_empty_a)  != 0) { perror("cond not_empty_a"); return -1; }

    /* --- Region B (x4) --- */
    shm->region_b_size = region_b_total_size(cfg->capacity_b);
    for (int i = 0; i < NUM_LEVELS; i++) {
        shm->region_b[i] = do_mmap(shm->region_b_size);
        if (!shm->region_b[i]) { perror("mmap region_b"); return -1; }

        region_b_t *rb = shm->region_b[i];
        rb->capacity = cfg->capacity_b;
        if (init_ps_mutex(&rb->level_mutex) != 0) { perror("mutex region_b"); return -1; }
        if (init_ps_cond(&rb->not_full_b)   != 0) { perror("cond not_full_b"); return -1; }
        if (init_ps_cond(&rb->not_empty_b)  != 0) { perror("cond not_empty_b"); return -1; }
    }

    /* --- Region C --- */
    shm->region_c_size = region_c_total_size();
    shm->region_c = do_mmap(shm->region_c_size);
    if (!shm->region_c) { perror("mmap region_c"); return -1; }

    region_c_t *rc = shm->region_c;
    if (init_ps_mutex(&rc->result_mutex) != 0) { perror("mutex region_c"); return -1; }
    if (init_ps_cond(&rc->result_cond)   != 0) { perror("cond region_c"); return -1; }
    for (int i = 0; i < NUM_LEVELS; i++) {
        if (sem_init(&rc->level_sem[i], 1, 0) != 0) { perror("sem_init"); return -1; }
        strncpy(rc->results[i].level, level_str(i), sizeof(rc->results[i].level) - 1);
    }

    /* --- Region D --- */
    shm->region_d_size = region_d_total_size(cfg->capacity_d);
    shm->region_d = do_mmap(shm->region_d_size);
    if (!shm->region_d) { perror("mmap region_d"); return -1; }

    region_d_t *rd = shm->region_d;
    rd->capacity = cfg->capacity_d;
    if (init_ps_mutex(&rd->priority_mutex) != 0) { perror("mutex region_d"); return -1; }
    if (init_ps_cond(&rd->not_full_d)      != 0) { perror("cond not_full_d"); return -1; }
    if (init_ps_cond(&rd->not_empty_d)     != 0) { perror("cond not_empty_d"); return -1; }

    return 0;
}

/* ---------- shm_destroy ---------- */
void shm_destroy(shm_handle_t *shm) {
    if (shm->region_a) {
        pthread_mutex_destroy(&shm->region_a->input_mutex);
        pthread_cond_destroy(&shm->region_a->not_full_a);
        pthread_cond_destroy(&shm->region_a->not_empty_a);
        munmap(shm->region_a, shm->region_a_size);
        shm->region_a = NULL;
    }
    for (int i = 0; i < NUM_LEVELS; i++) {
        if (shm->region_b[i]) {
            pthread_mutex_destroy(&shm->region_b[i]->level_mutex);
            pthread_cond_destroy(&shm->region_b[i]->not_full_b);
            pthread_cond_destroy(&shm->region_b[i]->not_empty_b);
            munmap(shm->region_b[i], shm->region_b_size);
            shm->region_b[i] = NULL;
        }
    }
    if (shm->region_c) {
        pthread_mutex_destroy(&shm->region_c->result_mutex);
        pthread_cond_destroy(&shm->region_c->result_cond);
        for (int i = 0; i < NUM_LEVELS; i++)
            sem_destroy(&shm->region_c->level_sem[i]);
        munmap(shm->region_c, shm->region_c_size);
        shm->region_c = NULL;
    }
    if (shm->region_d) {
        pthread_mutex_destroy(&shm->region_d->priority_mutex);
        pthread_cond_destroy(&shm->region_d->not_full_d);
        pthread_cond_destroy(&shm->region_d->not_empty_d);
        munmap(shm->region_d, shm->region_d_size);
        shm->region_d = NULL;
    }
}

/* ---------- Region A helpers ---------- */
void region_a_push_locked(region_a_t *ra, const log_entry_t *entry) {
    ra->buf[ra->tail] = *entry;
    ra->tail = (ra->tail + 1) % ra->capacity;
    ra->count++;
}

void region_a_pop_locked(region_a_t *ra, log_entry_t *entry) {
    *entry = ra->buf[ra->head];
    ra->head = (ra->head + 1) % ra->capacity;
    ra->count--;
}

/* ---------- Region B helpers ---------- */
void region_b_push_locked(region_b_t *rb, const log_entry_t *entry) {
    rb->buf[rb->tail] = *entry;
    rb->tail = (rb->tail + 1) % rb->capacity;
    rb->count++;
}

void region_b_pop_locked(region_b_t *rb, log_entry_t *entry) {
    *entry = rb->buf[rb->head];
    rb->head = (rb->head + 1) % rb->capacity;
    rb->count--;
}

/* ---------- Region D helpers ---------- */
void region_d_push_locked(region_d_t *rd, const log_entry_t *entry) {
    rd->buf[rd->tail] = *entry;
    rd->tail = (rd->tail + 1) % rd->capacity;
    rd->count++;
}

void region_d_pop_locked(region_d_t *rd, log_entry_t *entry) {
    *entry = rd->buf[rd->head];
    rd->head = (rd->head + 1) % rd->capacity;
    rd->count--;
}

/* ---------- Level utils ---------- */
int level_index(const char *str) {
    if (strcmp(str, "ERROR") == 0) return LVL_ERROR;
    if (strcmp(str, "WARN")  == 0) return LVL_WARN;
    if (strcmp(str, "INFO")  == 0) return LVL_INFO;
    if (strcmp(str, "DEBUG") == 0) return LVL_DEBUG;
    return -1;
}

const char *level_str(int lvl) {
    switch (lvl) {
        case LVL_ERROR: return "ERROR";
        case LVL_WARN:  return "WARN";
        case LVL_INFO:  return "INFO";
        case LVL_DEBUG: return "DEBUG";
        default:        return "UNKNOWN";
    }
}

int level_weight(int lvl) {
    switch (lvl) {
        case LVL_ERROR: return WEIGHT_ERROR;
        case LVL_WARN:  return WEIGHT_WARN;
        case LVL_INFO:  return WEIGHT_INFO;
        case LVL_DEBUG: return WEIGHT_DEBUG;
        default:        return 0;
    }
}

/* ---------- Overlapping keyword search ---------- */
long count_keyword_occurrences(const char *text, const char *keyword) {
    if (!text || !keyword || keyword[0] == '\0') return 0;
    size_t klen = strlen(keyword);
    long   count = 0;
    const char *p = text;
    while (*p) {
        if (strncmp(p, keyword, klen) == 0) {
            count++;
            p++;          /* sliding window: +1, not +klen */
        } else {
            p++;
        }
    }
    return count;
}
