#define _GNU_SOURCE
#include "watchdog.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>

void *watchdog_thread_func(void *arg) {
    watchdog_arg_t *wa = (watchdog_arg_t *)arg;
    time_t start_time  = time(NULL);
    long   elapsed     = 0;

    while (!(*wa->shutdown_flag)) {
        struct timeval tv = { .tv_sec = 3, .tv_usec = 0 };
        fd_set rset;
        int    maxfd = -1;

        FD_ZERO(&rset);
        for (int i = 0; i < wa->num_readers; i++) {
            if (wa->read_fds[i] >= 0) {
                FD_SET(wa->read_fds[i], &rset);
                if (wa->read_fds[i] > maxfd)
                    maxfd = wa->read_fds[i];
            }
        }

        if (maxfd < 0) {
            /* No open pipes — sleep and check flag */
            struct timespec ts = { .tv_sec = 3, .tv_nsec = 0 };
            nanosleep(&ts, NULL);
            elapsed = (long)(time(NULL) - start_time);
            if (*wa->shutdown_flag) break;
        } else {
            int ret = select(maxfd + 1, &rset, NULL, NULL, &tv);
            elapsed = (long)(time(NULL) - start_time);

            if (ret < 0) {
                if (errno == EINTR) continue;
                break;
            }

            /* Read available heartbeat lines */
            if (ret > 0) {
                for (int i = 0; i < wa->num_readers; i++) {
                    if (wa->read_fds[i] >= 0 && FD_ISSET(wa->read_fds[i], &rset)) {
                        char buf[256];
                        ssize_t n;
                        static char partial[MAX_READERS][512];
                        static int  partial_len[MAX_READERS];

                        while ((n = read(wa->read_fds[i], buf, sizeof(buf) - 1)) > 0) {
                            buf[n] = '\0';
                            /* Append to partial buffer */
                            int avail = (int)(sizeof(partial[i]) - 1 - partial_len[i]);
                            if (avail > 0) {
                                int copy = (n < avail) ? (int)n : avail;
                                memcpy(partial[i] + partial_len[i], buf, copy);
                                partial_len[i] += copy;
                                partial[i][partial_len[i]] = '\0';
                            }
                            /* Process complete lines */
                            char *p = partial[i];
                            char *nl;
                            while ((nl = strchr(p, '\n')) != NULL) {
                                *nl = '\0';
                                long lines = 0;
                                /* format: "[R<i>] <n> lines processed" */
                                int ri = 0;
                                if (sscanf(p, "[R%d] %ld lines processed", &ri, &lines) == 2) {
                                    if (ri >= 0 && ri < wa->num_readers)
                                        wa->lines_processed[ri] = lines;
                                }
                                p = nl + 1;
                            }
                            /* Keep remainder */
                            int rem = (int)(partial[i] + partial_len[i] - p);
                            if (rem > 0 && p != partial[i])
                                memmove(partial[i], p, rem);
                            partial_len[i] = rem;
                            partial[i][partial_len[i]] = '\0';

                            /* Check if pipe closed */
                            if (n == 0) {
                                close(wa->read_fds[i]);
                                wa->read_fds[i] = -1;
                                break;
                            }
                            /* Non-blocking: break if no more data */
                            break;
                        }
                        if (n == 0) {
                            close(wa->read_fds[i]);
                            wa->read_fds[i] = -1;
                        }
                    }
                }
            }
        }

        /* Count alive children (best-effort via WNOHANG) */
        int alive = 0;
        for (int i = 0; i < wa->num_readers; i++) {
            if (wa->read_fds[i] >= 0) alive++;
        }
        /* Print progress snapshot to stderr */
        fprintf(stderr, "[WATCHDOG] Progress at T+%lds:", elapsed);
        for (int i = 0; i < wa->num_readers; i++) {
            fprintf(stderr, " Reader%d=%ld", i, wa->lines_processed[i]);
        }
        fprintf(stderr, " children_alive=%d\n", wa->num_children);
        fflush(stderr);
    }

    return NULL;
}
