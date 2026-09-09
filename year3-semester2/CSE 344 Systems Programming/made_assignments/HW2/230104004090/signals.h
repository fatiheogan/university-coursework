#ifndef SIGNALS_H
#define SIGNALS_H

#include <signal.h>
#include "types.h"

/* Flags set inside signal handlers — must be volatile sig_atomic_t */
extern volatile sig_atomic_t g_worker_done  [MAX_WORKERS]; /* set when worker sends SIGUSR1 */
extern volatile sig_atomic_t g_workers_reaped;              /* number of reaped workers */
extern volatile sig_atomic_t g_sigint_received;             /* set when Ctrl+C is pressed */
extern volatile sig_atomic_t g_sigterm_worker;              /* set when worker receives SIGTERM */
extern volatile sig_atomic_t g_parent_sigterm;              /* set when parent is shutting down */

/* Worker PID table and match counts — shared with signal handlers */
extern pid_t g_worker_pids   [MAX_WORKERS];
extern int   g_worker_matches[MAX_WORKERS];
extern int   g_num_workers;

/* Install signal handlers for the parent process */
void install_parent_signals(void);

/* Install signal handlers for worker processes */
void install_worker_signals(void);

/* Returns 1 when all workers have finished (all done flags are set) */
int all_workers_done(int nw);

#endif /* SIGNALS_H */
