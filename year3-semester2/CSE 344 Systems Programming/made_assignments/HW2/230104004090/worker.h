#ifndef WORKER_H
#define WORKER_H

#include "types.h"

/*
 * run_worker – entry point for a forked child process.
 *
 * The worker:
 *   1. Installs SIGTERM/SIGINT handlers.
 *   2. Recursively searches all dirs in dirs[0..ndir-1].
 *   3. Prints each match to stdout in real time.
 *   4. Writes matches to tmp_path for the parent's tree builder.
 *   5. On SIGTERM: prints partial count and exits immediately.
 *   6. On normal finish: sends SIGUSR1 to parent, then exits.
 *
 * Exit status: match_count % 256
 * This function never returns — it always calls exit().
 */
void run_worker(char       **dirs,
                int          ndir,
                const char  *pattern,
                long         min_size,
                const char  *tmp_path);

#endif /* WORKER_H */
