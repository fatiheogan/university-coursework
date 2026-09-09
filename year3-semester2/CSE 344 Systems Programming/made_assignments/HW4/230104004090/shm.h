#ifndef SHM_H
#define SHM_H

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <time.h>

#define MAX_KEYWORDS    8
#define MAX_WORKERS     64
#define MAX_SOURCE_LEN  64
#define MAX_MSG_LEN     512
#define MAX_FILES       64
#define MAX_LINE_LEN    1024

/* Level indices */
#define LVL_ERROR  0
#define LVL_WARN   1
#define LVL_INFO   2
#define LVL_DEBUG  3
#define NUM_LEVELS 4

/* Level weights */
#define WEIGHT_ERROR 4
#define WEIGHT_WARN  3
#define WEIGHT_INFO  2
#define WEIGHT_DEBUG 1

/* Magic for binary output */
#define BINARY_MAGIC   0xC5E3440B
#define BINARY_VERSION 1

/* A single parsed log entry */
typedef struct {
    int  level;                    /* LVL_ERROR/WARN/INFO/DEBUG */
    int  is_eof;                   /* 1 = EOF marker for 'level' */
    char source[MAX_SOURCE_LEN];
    char message[MAX_MSG_LEN];
    char timestamp[32];
} log_entry_t;

/* ---- Region A: Dispatcher Input Queue ---- */
typedef struct {
    pthread_mutex_t input_mutex;
    pthread_cond_t  not_full_a;
    pthread_cond_t  not_empty_a;
    int             eof_count_per_level[NUM_LEVELS];
    int             total_readers;
    int             head;
    int             tail;
    int             count;
    int             capacity;
    log_entry_t     buf[1];  /* flexible – mmap provides real space */
} region_a_t;

/* ---- Region B: Per-level Analysis Buffer ---- */
typedef struct {
    pthread_mutex_t level_mutex;
    pthread_cond_t  not_full_b;
    pthread_cond_t  not_empty_b;
    int             eof_posted;
    int             head;
    int             tail;
    int             count;
    int             capacity;
    log_entry_t     buf[1];
} region_b_t;

/* ---- Region C: Results area ---- */
typedef struct {
    char   level[8];
    long   total_entries;
    double total_weighted_score;
    double per_keyword_score[MAX_KEYWORDS];
    double per_thread_score[MAX_WORKERS];
    char   top_source[3][MAX_SOURCE_LEN];
    long   top_source_hits[3];
    int    ready;
} level_result_t;

typedef struct {
    level_result_t  results[NUM_LEVELS];
    sem_t           level_sem[NUM_LEVELS];
    pthread_mutex_t result_mutex;
    pthread_cond_t  result_cond;
    int             results_ready; /* bitmask */
} region_c_t;

/* ---- Region D: High-Priority Buffer ---- */
typedef struct {
    pthread_mutex_t priority_mutex;
    pthread_cond_t  not_full_d;
    pthread_cond_t  not_empty_d;
    int             dispatcher_done;
    double          high_priority_score; /* written by aggregator after drain */
    int             head;
    int             tail;
    int             count;
    int             capacity;
    log_entry_t     buf[1];
} region_d_t;

/* ---- Shared memory handle (kept in parent; inherited by children) ---- */
typedef struct {
    region_a_t *region_a;
    size_t      region_a_size;

    region_b_t *region_b[NUM_LEVELS];
    size_t      region_b_size;   /* same for every level */

    region_c_t *region_c;
    size_t      region_c_size;

    region_d_t *region_d;
    size_t      region_d_size;
} shm_handle_t;

/* ---- Global config ---- */
typedef struct {
    int  capacity_a;
    int  capacity_b;
    int  capacity_d;
    int  num_files;
    int  num_keywords;
    int  num_reader_threads;
    int  num_worker_threads;
    int  timeout_sec;
    char keywords[MAX_KEYWORDS][64];
    char log_files[MAX_FILES][256];
    char filter_file[256];
    char output_file[256];
    char binary_file[256];
} config_t;

/* ---- Size helpers ---- */
size_t region_a_total_size(int capacity);
size_t region_b_total_size(int capacity);
size_t region_c_total_size(void);
size_t region_d_total_size(int capacity);

/* ---- Init / destroy ---- */
int  shm_init(shm_handle_t *shm, const config_t *cfg);
void shm_destroy(shm_handle_t *shm);

/* ---- Region A helpers (caller must hold lock) ---- */
void region_a_push_locked(region_a_t *ra, const log_entry_t *entry);
void region_a_pop_locked(region_a_t *ra, log_entry_t *entry);

/* ---- Region B helpers (caller must hold lock) ---- */
void region_b_push_locked(region_b_t *rb, const log_entry_t *entry);
void region_b_pop_locked(region_b_t *rb, log_entry_t *entry);

/* ---- Region D helpers (caller must hold lock) ---- */
void region_d_push_locked(region_d_t *rd, const log_entry_t *entry);
void region_d_pop_locked(region_d_t *rd, log_entry_t *entry);

/* ---- Utility ---- */
int        level_index(const char *str);
const char *level_str(int lvl);
int        level_weight(int lvl);

/* ---- Keyword search (overlapping) ---- */
long count_keyword_occurrences(const char *text, const char *keyword);

#endif /* SHM_H */
