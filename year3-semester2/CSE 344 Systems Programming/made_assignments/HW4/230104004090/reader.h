#ifndef READER_H
#define READER_H

#include "shm.h"

typedef struct {
    int          reader_index;
    const char  *filename;
    int          num_threads;
    int          heartbeat_fd;   /* write-end of pipe to parent */
    shm_handle_t *shm;
    const config_t *cfg;
} reader_arg_t;

/* Entry point called by child process after fork */
void reader_process_main(int reader_index, const char *filename,
                         int num_threads, int heartbeat_fd,
                         shm_handle_t *shm, const config_t *cfg);

#endif /* READER_H */
