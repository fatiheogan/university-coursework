#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <signal.h>
#include "shm.h"

#define MAX_READERS MAX_FILES

typedef struct {
    int    read_fds[MAX_READERS];  /* pipe read-ends from Reader processes */
    int    num_readers;
    long   lines_processed[MAX_READERS];
    char   file_names[MAX_READERS][256];
    int    num_children;           /* current alive children (approximate) */
    volatile sig_atomic_t *shutdown_flag;
} watchdog_arg_t;

void *watchdog_thread_func(void *arg);

#endif /* WATCHDOG_H */
