#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include "signals.h"

/* Flags for tracking signal events across parent and workers */
volatile sig_atomic_t g_worker_done   [MAX_WORKERS];
volatile sig_atomic_t g_workers_reaped  = 0;
volatile sig_atomic_t g_sigint_received = 0;
volatile sig_atomic_t g_sigterm_worker  = 0;
volatile sig_atomic_t g_parent_sigterm  = 0;

/* SIGUSR1 handler: marks the sending worker as done using its PID */
static void handle_sigusr1(int sig, siginfo_t *info, void *ctx)
{
    (void)sig; (void)ctx;
    pid_t sender = info->si_pid;
    /* Find the matching worker slot and set its done flag */
    for (int i = 0; i < g_num_workers; i++) {
        if (g_worker_pids[i] == sender) {
            g_worker_done[i] = 1;
            break;
        }
    }
}

/* SIGCHLD handler: reaps zombie processes and logs unexpected crashes */
static void handle_sigchld(int sig)
{
    (void)sig;
    int   saved_errno = errno;
    int   status;
    pid_t pid;

    /* Reap all finished children without blocking */
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        g_workers_reaped++;

        /* Save exit status for the matching worker */
        for (int i = 0; i < g_num_workers; i++) {
            if (g_worker_pids[i] == pid) {
                if (WIFEXITED(status))
                    g_worker_matches[i] = WEXITSTATUS(status);
                break;
            }
        }

        /* Log only if killed by a signal (real crash), not a normal exit */
        if (!g_parent_sigterm && WIFSIGNALED(status)) {
            /* Use write() only — printf is not async-signal-safe */
            const char pre[]  = "[Parent] Worker PID:";
            const char mid[]  = " terminated unexpectedly (exit status: ";
            const char suf[]  = ").\n";
            char pidstr[16], sigstr[16];
            int tmp, pos;

            /* Convert pid to string manually */
            tmp = (int)pid; pos = 15; pidstr[pos] = '\0';
            if (tmp == 0) { pidstr[--pos] = '0'; }
            else { while (tmp > 0) { pidstr[--pos] = '0' + (tmp % 10); tmp /= 10; } }

            /* Convert signal number to string manually */
            tmp = WTERMSIG(status); pos = 15; sigstr[pos] = '\0';
            if (tmp == 0) { sigstr[--pos] = '0'; }
            else { while (tmp > 0) { sigstr[--pos] = '0' + (tmp % 10); tmp /= 10; } }

            write(STDERR_FILENO, pre,          sizeof(pre)  - 1);
            write(STDERR_FILENO, pidstr + pos, 15 - pos);
            write(STDERR_FILENO, mid,          sizeof(mid)  - 1);
            write(STDERR_FILENO, sigstr + pos, 15 - pos);
            write(STDERR_FILENO, suf,          sizeof(suf)  - 1);
        }
    }
    errno = saved_errno;
}

/* SIGINT handler: sets flag so parent can start shutdown in main loop */
static void handle_sigint(int sig)
{
    (void)sig;
    g_sigint_received = 1;
}

/* SIGTERM handler for workers: sets flag to stop crawling and exit cleanly */
static void handle_sigterm_worker(int sig)
{
    (void)sig;
    g_sigterm_worker = 1;
}

/* Returns 1 if all workers have set their done flag, 0 otherwise */
int all_workers_done(int nw)
{
    for (int i = 0; i < nw; i++)
        if (!g_worker_done[i]) return 0;
    return 1;
}

/* Installs SIGUSR1, SIGCHLD, and SIGINT handlers for the parent process */
void install_parent_signals(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    /* SIGUSR1: use SA_SIGINFO to get the sender PID */
    sa.sa_sigaction = handle_sigusr1;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGCHLD);
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &sa, NULL);

    /* SIGCHLD: reap zombie children */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    /* SIGINT: catch Ctrl+C */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
}

/* Installs signal handlers for worker processes */
void install_worker_signals(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    /* SIGTERM: worker stops crawling and exits cleanly */
    sa.sa_handler = handle_sigterm_worker;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);

    /* SIGINT: workers ignore Ctrl+C, parent handles it */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT,  &sa, NULL);

    /* SIGCHLD: restore default behavior in workers */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);

    /* SIGUSR1: workers ignore this signal */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
}
