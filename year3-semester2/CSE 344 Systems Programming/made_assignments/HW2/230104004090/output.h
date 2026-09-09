#ifndef OUTPUT_H
#define OUTPUT_H

#include "types.h"

/* Global storage for all match results and counters */
extern MatchResult g_results    [MAX_MATCHES];
extern int         g_result_cnt;
extern int         g_total_scan;

/* Reads worker temp files and fills g_results */
void collect_results(int         nw,
                     char        tmp_paths[][64],
                     pid_t      *pids);

/* Prints matched files as an indented directory tree */
void print_tree(const char *root);

/* Prints total workers, files scanned, and per-worker match counts */
void print_summary(int    nw,
                   pid_t *worker_pids);

#endif /* OUTPUT_H */
