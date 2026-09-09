#ifndef TYPES_H
#define TYPES_H

#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>   /* pid_t */

/* Compile-time limits used across the whole program */
#define MAX_WORKERS   8       /* maximum number of worker processes */
#define MAX_PATH      4096    /* maximum file path length */
#define MAX_SUBDIRS   1024    /* maximum subdirectories under root */
#define MAX_MATCHES   65536   /* maximum total matched files */
#define MAX_PRINTED   512     /* unique dirs tracked during tree output */

/* Holds info about a single matched file */
typedef struct {
    char  path[MAX_PATH];   /* full path to the matched file */
    long  size;             /* file size in bytes */
    pid_t worker_pid;       /* PID of the worker that found it */
} MatchResult;

#endif /* TYPES_H */
