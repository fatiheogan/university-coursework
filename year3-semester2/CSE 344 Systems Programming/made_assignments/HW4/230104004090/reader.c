#define _GNU_SOURCE
#include "reader.h"
#include "shm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <errno.h>

/* ---------- Internal bounded buffer (private to one Reader) ---------- */
#define INTERNAL_BUF_CAP 128

typedef struct {
    log_entry_t     entries[INTERNAL_BUF_CAP];
    int             head, tail, count;
    pthread_mutex_t mutex;    /* default (non-process-shared) */
    pthread_cond_t  not_full;
    pthread_cond_t  not_empty;
    int             done;     /* all reader threads finished */
    int             writers;  /* how many reader threads are still active */
} internal_buf_t;

/* ---------- Arguments for reader thread ---------- */
typedef struct {
    int              thread_idx;
    int              reader_idx;
    const char      *filename;
    off_t            range_start;
    off_t            range_end;   /* exclusive */
    internal_buf_t  *ibuf;
    int              heartbeat_fd;
    long             lines_read;
    long             malformed;
} reader_thread_arg_t;

/* ---------- Arguments for parser thread ---------- */
typedef struct {
    int              reader_idx;
    internal_buf_t  *ibuf;
    shm_handle_t    *shm;
    const config_t  *cfg;
    long             dispatched[NUM_LEVELS];
} parser_thread_arg_t;

/* ---- Parse one log line into entry; return 0 ok, -1 skip ---- */
static int parse_log_line(const char *line, log_entry_t *entry) {
    /* Format: [YYYY-MM-DD HH:MM:SS] [LEVEL] [SOURCE] MESSAGE */
    if (!line || line[0] != '[') return -1;

    /* Timestamp */
    const char *p = line + 1;
    const char *end = strchr(p, ']');
    if (!end) return -1;
    size_t ts_len = (size_t)(end - p);
    if (ts_len >= sizeof(entry->timestamp)) return -1;
    memcpy(entry->timestamp, p, ts_len);
    entry->timestamp[ts_len] = '\0';
    p = end + 1;

    /* Skip space */
    if (*p != ' ') return -1;
    p++;

    /* Level */
    if (*p != '[') return -1;
    p++;
    end = strchr(p, ']');
    if (!end) return -1;
    char lvl_str[16];
    size_t lvl_len = (size_t)(end - p);
    if (lvl_len >= sizeof(lvl_str)) return -1;
    memcpy(lvl_str, p, lvl_len);
    lvl_str[lvl_len] = '\0';
    entry->level = level_index(lvl_str);
    if (entry->level < 0) return -1;
    p = end + 1;

    /* Skip space */
    if (*p != ' ') return -1;
    p++;

    /* Source */
    if (*p != '[') return -1;
    p++;
    end = strchr(p, ']');
    if (!end) return -1;
    size_t src_len = (size_t)(end - p);
    if (src_len >= MAX_SOURCE_LEN) return -1;
    memcpy(entry->source, p, src_len);
    entry->source[src_len] = '\0';
    p = end + 1;

    /* Strip leading space from message */
    while (*p == ' ') p++;
    strncpy(entry->message, p, MAX_MSG_LEN - 1);
    entry->message[MAX_MSG_LEN - 1] = '\0';
    /* Remove trailing newline */
    size_t mlen = strlen(entry->message);
    while (mlen > 0 && (entry->message[mlen-1] == '\n' || entry->message[mlen-1] == '\r'))
        entry->message[--mlen] = '\0';

    entry->is_eof = 0;
    return 0;
}

/* ---- Reader thread ---- */
static void *reader_thread_func(void *varg) {
    reader_thread_arg_t *arg = (reader_thread_arg_t *)varg;
    pid_t tid = (pid_t)syscall(SYS_gettid);
    printf("[PID:%d][TID:%d] Reader thread %d: range [%ld, %ld) bytes\n",
           (int)getpid(), (int)tid, arg->thread_idx,
           (long)arg->range_start, (long)arg->range_end);
    fflush(stdout);

    FILE *f = fopen(arg->filename, "r");
    if (!f) {
        fprintf(stderr, "Reader thread %d: cannot open %s: %s\n",
                arg->thread_idx, arg->filename, strerror(errno));
        pthread_mutex_lock(&arg->ibuf->mutex);
        arg->ibuf->writers--;
        if (arg->ibuf->writers == 0) {
            arg->ibuf->done = 1;
            pthread_cond_broadcast(&arg->ibuf->not_empty);
        }
        pthread_mutex_unlock(&arg->ibuf->mutex);
        return NULL;
    }

    /* Seek to range_start; if not 0, skip to next newline boundary */
    if (arg->range_start > 0) {
        if (fseeko(f, arg->range_start, SEEK_SET) != 0) {
            fclose(f);
            pthread_mutex_lock(&arg->ibuf->mutex);
            arg->ibuf->writers--;
            if (arg->ibuf->writers == 0) {
                arg->ibuf->done = 1;
                pthread_cond_broadcast(&arg->ibuf->not_empty);
            }
            pthread_mutex_unlock(&arg->ibuf->mutex);
            return NULL;
        }
        /* Skip partial first line */
        char skip[MAX_LINE_LEN];
        if (!fgets(skip, sizeof(skip), f)) {
            /* EOF already */
            fclose(f);
            pthread_mutex_lock(&arg->ibuf->mutex);
            arg->ibuf->writers--;
            if (arg->ibuf->writers == 0) {
                arg->ibuf->done = 1;
                pthread_cond_broadcast(&arg->ibuf->not_empty);
            }
            pthread_mutex_unlock(&arg->ibuf->mutex);
            return NULL;
        }
    }

    long heartbeat_count = 0;
    char linebuf[MAX_LINE_LEN];

    while (1) {
        off_t pos = ftello(f);
        /* If range_end != -1 and we're past it, stop */
        if (arg->range_end != (off_t)-1 && pos >= arg->range_end) break;

        if (!fgets(linebuf, sizeof(linebuf), f)) break;

        /* Skip blank lines */
        if (linebuf[0] == '\n' || linebuf[0] == '\r' || linebuf[0] == '\0') continue;

        log_entry_t entry;
        memset(&entry, 0, sizeof(entry));
        if (parse_log_line(linebuf, &entry) != 0) {
            arg->malformed++;
            continue;
        }
        arg->lines_read++;

        /* Push to internal buffer */
        pthread_mutex_lock(&arg->ibuf->mutex);
        while (arg->ibuf->count >= INTERNAL_BUF_CAP)
            pthread_cond_wait(&arg->ibuf->not_full, &arg->ibuf->mutex);
        arg->ibuf->entries[arg->ibuf->tail] = entry;
        arg->ibuf->tail = (arg->ibuf->tail + 1) % INTERNAL_BUF_CAP;
        arg->ibuf->count++;
        pthread_cond_signal(&arg->ibuf->not_empty);
        pthread_mutex_unlock(&arg->ibuf->mutex);

        /* Heartbeat every 50 lines */
        heartbeat_count++;
        if (heartbeat_count % 50 == 0) {
            char hb[128];
            int hblen = snprintf(hb, sizeof(hb), "[R%d] %ld lines processed\n",
                                 arg->reader_idx, arg->lines_read);
            write(arg->heartbeat_fd, hb, hblen);
        }
    }

    fclose(f);

    /* Final heartbeat */
    {
        char hb[128];
        int hblen = snprintf(hb, sizeof(hb), "[R%d] %ld lines processed\n",
                             arg->reader_idx, arg->lines_read);
        write(arg->heartbeat_fd, hb, hblen);
    }

    printf("[PID:%d][TID:%d] Reader thread %d: finished, lines_read=%ld, malformed=%ld\n",
           (int)getpid(), (int)tid, arg->thread_idx,
           arg->lines_read, arg->malformed);
    fflush(stdout);

    pthread_mutex_lock(&arg->ibuf->mutex);
    arg->ibuf->writers--;
    if (arg->ibuf->writers == 0) {
        arg->ibuf->done = 1;
        pthread_cond_broadcast(&arg->ibuf->not_empty);
    }
    pthread_mutex_unlock(&arg->ibuf->mutex);

    return NULL;
}

/* ---- Parser thread: consumes ibuf, pushes to Region A ---- */
static void *parser_thread_func(void *varg) {
    parser_thread_arg_t *arg  = (parser_thread_arg_t *)varg;
    internal_buf_t      *ibuf = arg->ibuf;
    region_a_t          *ra   = arg->shm->region_a;
    int cap_a = arg->cfg->capacity_a;

    while (1) {
        pthread_mutex_lock(&ibuf->mutex);
        while (ibuf->count == 0 && !ibuf->done)
            pthread_cond_wait(&ibuf->not_empty, &ibuf->mutex);

        if (ibuf->count == 0 && ibuf->done) {
            pthread_mutex_unlock(&ibuf->mutex);
            break;
        }

        log_entry_t entry = ibuf->entries[ibuf->head];
        ibuf->head = (ibuf->head + 1) % INTERNAL_BUF_CAP;
        ibuf->count--;
        pthread_cond_signal(&ibuf->not_full);
        pthread_mutex_unlock(&ibuf->mutex);

        /* Push to Region A */
        pthread_mutex_lock(&ra->input_mutex);
        while (ra->count >= cap_a)
            pthread_cond_wait(&ra->not_full_a, &ra->input_mutex);
        region_a_push_locked(ra, &entry);
        pthread_cond_signal(&ra->not_empty_a);
        pthread_mutex_unlock(&ra->input_mutex);

        arg->dispatched[entry.level]++;
    }

    /* Send one EOF marker per level */
    for (int lvl = 0; lvl < NUM_LEVELS; lvl++) {
        log_entry_t eof_entry;
        memset(&eof_entry, 0, sizeof(eof_entry));
        eof_entry.is_eof = 1;
        eof_entry.level  = lvl;

        pthread_mutex_lock(&ra->input_mutex);
        while (ra->count >= cap_a)
            pthread_cond_wait(&ra->not_full_a, &ra->input_mutex);
        region_a_push_locked(ra, &eof_entry);
        ra->eof_count_per_level[lvl]++;   /* spec: incremented by parser thread */
        pthread_cond_signal(&ra->not_empty_a);
        pthread_mutex_unlock(&ra->input_mutex);
    }

    printf("[PID:%d] Parser thread: dispatched E:%ld W:%ld I:%ld D:%ld -> Region A\n",
           (int)getpid(),
           arg->dispatched[LVL_ERROR],
           arg->dispatched[LVL_WARN],
           arg->dispatched[LVL_INFO],
           arg->dispatched[LVL_DEBUG]);
    fflush(stdout);

    return NULL;
}

/* ---------- Reader process main ---------- */
void reader_process_main(int reader_index, const char *filename,
                         int num_threads, int heartbeat_fd,
                         shm_handle_t *shm, const config_t *cfg) {
    printf("[PID:%d] Reader %d started. File: %s, Threads: %d\n",
           (int)getpid(), reader_index, filename, num_threads);
    fflush(stdout);

    /* Get file size */
    struct stat st;
    if (stat(filename, &st) != 0) {
        fprintf(stderr, "[PID:%d] Reader %d: cannot stat %s: %s\n",
                (int)getpid(), reader_index, filename, strerror(errno));
        _exit(1);
    }
    off_t file_size = st.st_size;

    /* Divide file into num_threads ranges */
    off_t chunk = (file_size + num_threads - 1) / num_threads;
    if (chunk == 0) chunk = 1;

    /* Internal buffer */
    internal_buf_t ibuf;
    memset(&ibuf, 0, sizeof(ibuf));
    pthread_mutex_init(&ibuf.mutex, NULL);
    pthread_cond_init(&ibuf.not_full, NULL);
    pthread_cond_init(&ibuf.not_empty, NULL);
    ibuf.writers = num_threads;

    /* Launch reader threads */
    pthread_t *rtids = calloc(num_threads, sizeof(pthread_t));
    reader_thread_arg_t *rargs = calloc(num_threads, sizeof(reader_thread_arg_t));

    for (int i = 0; i < num_threads; i++) {
        rargs[i].thread_idx   = i;
        rargs[i].reader_idx   = reader_index;
        rargs[i].filename     = filename;
        rargs[i].range_start  = (off_t)i * chunk;
        rargs[i].range_end    = (i == num_threads - 1) ? (off_t)-1
                                                       : (off_t)(i + 1) * chunk;
        rargs[i].ibuf         = &ibuf;
        rargs[i].heartbeat_fd = heartbeat_fd;
        rargs[i].lines_read   = 0;
        rargs[i].malformed    = 0;

        if (pthread_create(&rtids[i], NULL, reader_thread_func, &rargs[i]) != 0) {
            fprintf(stderr, "pthread_create reader_thread failed\n");
            _exit(1);
        }
    }

    /* Launch parser thread */
    parser_thread_arg_t parg;
    memset(&parg, 0, sizeof(parg));
    parg.reader_idx = reader_index;
    parg.ibuf       = &ibuf;
    parg.shm        = shm;
    parg.cfg        = cfg;

    pthread_t ptid;
    if (pthread_create(&ptid, NULL, parser_thread_func, &parg) != 0) {
        fprintf(stderr, "pthread_create parser_thread failed\n");
        _exit(1);
    }

    /* Wait for all reader threads */
    for (int i = 0; i < num_threads; i++)
        pthread_join(rtids[i], NULL);

    /* Wait for parser thread */
    pthread_join(ptid, NULL);

    /* Cleanup */
    pthread_mutex_destroy(&ibuf.mutex);
    pthread_cond_destroy(&ibuf.not_full);
    pthread_cond_destroy(&ibuf.not_empty);
    free(rtids);
    free(rargs);

    printf("[PID:%d] Reader %d exiting.\n", (int)getpid(), reader_index);
    fflush(stdout);
    _exit(0);
}
