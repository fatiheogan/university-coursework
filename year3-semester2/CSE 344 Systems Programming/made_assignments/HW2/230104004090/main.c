#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>

#include "types.h"
#include "signals.h"
#include "pattern.h"
#include "dirutils.h"
#include "worker.h"
#include "output.h"

/* Global worker PID table and match counts — shared with signals.c */
pid_t g_worker_pids   [MAX_WORKERS];
int   g_worker_matches[MAX_WORKERS];
int   g_num_workers   = 0;

/* Temp file paths — one file per worker to store match results */
static char g_tmp_paths[MAX_WORKERS][64];

/* Handles SIGINT: sends SIGTERM to all workers, waits, then SIGKILL if needed */
static void do_sigint_cleanup(int nw)
{
    const char msg[] = "[Parent] SIGINT received. Terminating all workers...\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);

    g_parent_sigterm = 1;

    /* Step 1: send SIGTERM to all live workers */
    for (int i = 0; i < nw; i++)
        if (g_worker_pids[i] > 0)
            kill(g_worker_pids[i], SIGTERM);

    /* Step 2: wait up to 3 seconds for workers to exit gracefully */
    int deadline = 30;   /* 30 × 100 ms = 3 s */
    while (deadline-- > 0) {
        int all_gone = 1;
        for (int i = 0; i < nw; i++) {
            if (g_worker_pids[i] > 0 &&
                kill(g_worker_pids[i], 0) == 0)   /* still alive? */
            {
                all_gone = 0;
                break;
            }
        }
        if (all_gone) break;

        struct timespec ts = {0, 100000000L};  /* sleep 100 ms */
        nanosleep(&ts, NULL);
    }

    /* Step 3: force-kill any workers that are still running */
    for (int i = 0; i < nw; i++) {
        if (g_worker_pids[i] <= 0) continue;

        if (kill(g_worker_pids[i], 0) == 0)
            kill(g_worker_pids[i], SIGKILL);

        /* Block until this child is fully reaped */
        int status;
        while (waitpid(g_worker_pids[i], &status, 0) < 0 && errno == EINTR)
            ;
    }

    /* Remove all temp files */
    for (int i = 0; i < nw; i++) unlink(g_tmp_paths[i]);

    /* Print a partial summary with whatever was collected before interrupt */
    int total = 0;
    for (int i = 0; i < nw; i++) total += g_worker_matches[i];
    printf("--- Partial Summary (interrupted) ---\n");
    printf("Partial matches collected : %d\n", total);
}

/* Program entry point */
int main(int argc, char *argv[])
{
    char *root_dir = NULL;
    int   nw_req   = 0;
    char *pattern  = NULL;
    long  min_size = 0;
    int   opt;

    /* Parse command-line arguments with getopt */
    while ((opt = getopt(argc, argv, "d:n:f:s:")) != -1) {
        switch (opt) {
        case 'd': root_dir = optarg;        break;
        case 'n': nw_req   = atoi(optarg);  break;
        case 'f': pattern  = optarg;        break;
        case 's': min_size = atol(optarg);  break;
        default:
            fprintf(stderr,
                "Usage: %s -d <root_dir> -n <num_workers> -f <pattern> "
                "[-s <min_size_bytes>]\n", argv[0]);
            return 1;
        }
    }

    /* Validate required arguments */
    if (!root_dir || !pattern || nw_req < 2 || nw_req > MAX_WORKERS) {
        fprintf(stderr,
            "Usage: %s -d <root_dir> -n <num_workers> -f <pattern> "
            "[-s <min_size_bytes>]\n"
            "  -n must be between 2 and %d (inclusive)\n",
            argv[0], MAX_WORKERS);
        return 1;
    }

    /* Check that root_dir exists and is a directory */
    struct stat st_root;
    if (stat(root_dir, &st_root) < 0 || !S_ISDIR(st_root.st_mode)) {
        fprintf(stderr, "Error: '%s' is not a valid directory\n", root_dir);
        return 1;
    }

    /* Get immediate subdirectories of root */
    int    n_subdirs;
    char **subdirs = get_subdirs(root_dir, &n_subdirs);

    /* Edge case: root has no subdirectories — parent searches directly */
    if (n_subdirs == 0) {
        printf("Notice: no subdirectories found; parent will search root directly.\n");
        direct_search(root_dir, pattern, min_size,
                      g_results, &g_result_cnt, &g_total_scan);
        putchar('\n');
        print_tree(root_dir);
        putchar('\n');
        printf("--- Summary ---\n");
        printf("Total workers used : 0\n");
        printf("Total files scanned : %d\n", g_total_scan);
        printf("Total matches found : %d\n", g_result_cnt);
        free(subdirs);
        return 0;
    }

    /* Reduce worker count if there are fewer subdirs than requested */
    int actual_nw = nw_req;
    if (n_subdirs < nw_req) {
        printf("Notice: only %d subdirectories found; using %d workers instead of %d.\n",
               n_subdirs, n_subdirs, nw_req);
        actual_nw = n_subdirs;
    }
    g_num_workers = actual_nw;

    /* Distribute subdirectories to workers in round-robin order */
    char **wdirs   [MAX_WORKERS];
    int    wdir_cnt[MAX_WORKERS];
    memset(wdir_cnt, 0, sizeof(wdir_cnt));

    for (int i = 0; i < actual_nw; i++) {
        wdirs[i] = malloc(MAX_SUBDIRS * sizeof(char *));
        if (!wdirs[i]) { perror("malloc"); return 1; }
    }
    for (int i = 0; i < n_subdirs; i++)
        wdirs[i % actual_nw][wdir_cnt[i % actual_nw]++] = subdirs[i];

    /* Build unique temp file path for each worker */
    pid_t ppid = getpid();
    for (int i = 0; i < actual_nw; i++)
        snprintf(g_tmp_paths[i], sizeof(g_tmp_paths[i]),
                 "/tmp/.procSearch_%d_%d", (int)ppid, i);

    /* Install parent signal handlers before forking */
    install_parent_signals();

    /* Flush stdout so buffered output is not duplicated in child processes */
    fflush(stdout);

    /* Block SIGCHLD while forking so all PIDs are recorded before any handler fires */
    sigset_t block_chld, old_mask;
    sigemptyset(&block_chld);
    sigaddset(&block_chld, SIGCHLD);
    sigprocmask(SIG_BLOCK, &block_chld, &old_mask);

    for (int i = 0; i < actual_nw; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            sigprocmask(SIG_SETMASK, &old_mask, NULL);
            /* Kill already-forked workers on error */
            for (int j = 0; j < i; j++) kill(g_worker_pids[j], SIGKILL);
            return 1;
        }
        if (pid == 0) {
            /* Child process: restore signal mask and start working */
            sigprocmask(SIG_SETMASK, &old_mask, NULL);
            run_worker(wdirs[i], wdir_cnt[i], pattern, min_size, g_tmp_paths[i]);
            /* run_worker always calls exit() — never reaches here */
        }
        g_worker_pids[i] = pid;
    }

    /* All PIDs saved — unblock SIGCHLD */
    sigprocmask(SIG_SETMASK, &old_mask, NULL);

    /* Parent waits until all workers are done, all are reaped, or Ctrl+C */
    while (!all_workers_done(actual_nw)
        && g_workers_reaped < actual_nw
        && !g_sigint_received)
        pause();

    /* Handle Ctrl+C: terminate workers and print partial summary */
    if (g_sigint_received) {
        do_sigint_cleanup(actual_nw);
        for (int i = 0; i < actual_nw; i++) free(wdirs[i]);
        for (int i = 0; i < n_subdirs;  i++) free(subdirs[i]);
        free(subdirs);
        return 0;
    }

    /* Collect exit statuses from any workers not yet reaped by SIGCHLD */
    for (int i = 0; i < actual_nw; i++) {
        int   status;
        pid_t res = waitpid(g_worker_pids[i], &status, WNOHANG);
        if (res == 0) {
            /* Worker not finished yet — wait for it */
            while (waitpid(g_worker_pids[i], &status, 0) < 0 && errno == EINTR)
                ;
            res = g_worker_pids[i];
        }
        /* Only update if SIGCHLD handler hasn't already set it */
        if (res > 0 && WIFEXITED(status))
            g_worker_matches[i] = WEXITSTATUS(status);
    }

    /* Read each worker's temp file and fill g_results */
    collect_results(actual_nw, g_tmp_paths, g_worker_pids);

    /* Print tree view and summary */
    putchar('\n');
    print_tree(root_dir);
    putchar('\n');
    print_summary(actual_nw, g_worker_pids);

    /* Free all allocated memory */
    for (int i = 0; i < actual_nw; i++) free(wdirs[i]);
    for (int i = 0; i < n_subdirs;  i++) free(subdirs[i]);
    free(subdirs);

    return 0;
}
