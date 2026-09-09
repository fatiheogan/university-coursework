#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <semaphore.h>

/* ── Logging macros ── */

/* Print an info message prefixed with the caller's PID */
#define LOG(fmt, ...) \
    printf("[PID:%d] " fmt "\n", getpid(), ##__VA_ARGS__)

/* Print an error message to stderr prefixed with the caller's PID */
#define ERRLOG(fmt, ...) \
    fprintf(stderr, "ERROR [PID:%d] " fmt "\n", getpid(), ##__VA_ARGS__)

/* ── Timing helpers ── */

/* Sleep for the given number of milliseconds */
static inline void sleep_ms(int ms) {
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

/* sem_wait with a relative timeout of ms milliseconds.
   Returns 0 on success, -1 on timeout or error (errno set). */
static inline int sem_twait(sem_t *sem, int ms) {
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) < 0) return -1;
    ts.tv_nsec += (long)ms * 1000000L;
    /* Carry nanoseconds overflow into seconds */
    while (ts.tv_nsec >= 1000000000L) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000L;
    }
    return sem_timedwait(sem, &ts);
}

/* ── Error-checked wrappers ── */

/* Call a function and exit with error if it returns < 0 */
#define CHKRET(call, msg)                                        \
    do {                                                         \
        if ((call) < 0) {                                        \
            perror(msg);                                         \
            exit(EXIT_FAILURE);                                  \
        }                                                        \
    } while (0)

/* Initialise a shared semaphore; exit on failure */
#define SEM_INIT(sem, val)                                       \
    do {                                                         \
        if (sem_init((sem), 1, (val)) != 0) {                   \
            perror("sem_init");                                  \
            exit(EXIT_FAILURE);                                  \
        }                                                        \
    } while (0)

/* sem_wait that retries on EINTR and exits on any other error */
static inline void safe_sem_wait(sem_t *s, const char *name) {
    while (sem_wait(s) != 0) {
        if (errno == EINTR) continue;  /* interrupted by signal, retry */
        perror(name);
        exit(EXIT_FAILURE);
    }
}

/* Convenience macros for safe wait and post */
#define SWAIT(s)  safe_sem_wait((s), #s)
#define SPOST(s)  do { if (sem_post(s) != 0) { perror(#s); } } while (0)

#endif /* UTILS_H */
