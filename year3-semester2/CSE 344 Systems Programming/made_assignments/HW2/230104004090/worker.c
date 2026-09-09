#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "worker.h"
#include "signals.h"
#include "pattern.h"

/* Worker-local counters and temp file descriptor (not shared with parent) */
static int w_match_count = 0;
static int w_scan_count  = 0;
static int w_tmp_fd      = -1;

/* Recursively searches dir for matching files; stops if SIGTERM is received */
static void w_search(const char *dir, const char *pattern, long min_size)
{
    if (g_sigterm_worker) return;

    DIR *d = opendir(dir);
    if (!d) return;

    struct dirent *e;
    struct stat    st;
    char           fullpath[MAX_PATH * 2];

    while ((e = readdir(d)) != NULL) {
        if (g_sigterm_worker) break;
        if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;

        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, e->d_name);
        if (lstat(fullpath, &st) < 0) continue;

        if (S_ISDIR(st.st_mode)) {
            /* Recurse into subdirectory */
            w_search(fullpath, pattern, min_size);
        } else if (S_ISREG(st.st_mode)) {
            w_scan_count++;
            /* Skip if file is smaller than min_size */
            if (min_size > 0 && st.st_size < min_size) continue;
            if (pattern_match(pattern, e->d_name)) {
                w_match_count++;

                /* Print match to stdout in real time */
                printf("[Worker PID:%d] MATCH: %s (%ld bytes)\n",
                       (int)getpid(), fullpath, (long)st.st_size);
                fflush(stdout);

                /* Write path and size to temp file for parent's tree builder */
                if (w_tmp_fd >= 0) {
                    char rec[MAX_PATH * 2 + 32];
                    int  rlen = snprintf(rec, sizeof(rec),
                                         "%s\t%ld\n", fullpath, (long)st.st_size);
                    write(w_tmp_fd, rec, rlen);
                }
            }
        }
    }
    closedir(d);
}

/* Entry point for forked worker: searches assigned dirs, notifies parent when done */
void run_worker(char       **dirs,
                int          ndir,
                const char  *pattern,
                long         min_size,
                const char  *tmp_path)
{
    /* Set up signal handlers for this worker */
    install_worker_signals();

    /* Open temp file and write PID header so parent knows which worker wrote it */
    w_tmp_fd = open(tmp_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (w_tmp_fd >= 0) {
        char hdr[64];
        int  hlen = snprintf(hdr, sizeof(hdr), "#PID %d\n", (int)getpid());
        write(w_tmp_fd, hdr, hlen);
    }

    /* Search all assigned directories until done or SIGTERM received */
    for (int i = 0; i < ndir && !g_sigterm_worker; i++)
        w_search(dirs[i], pattern, min_size);

    /* Write total scanned count footer and close the temp file */
    if (w_tmp_fd >= 0) {
        char ftr[64];
        int  flen = snprintf(ftr, sizeof(ftr), "#SCANNED %d\n", w_scan_count);
        write(w_tmp_fd, ftr, flen);
        close(w_tmp_fd);
        w_tmp_fd = -1;
    }

    /* If stopped early by SIGTERM, print partial count and exit */
    if (g_sigterm_worker) {
        printf("[Worker PID:%d] SIGTERM received. Partial matches: %d. Exiting.\n",
               (int)getpid(), w_match_count);
        fflush(stdout);
        exit(w_match_count % 256);
    }

    /* Normal finish: send SIGUSR1 to parent, then exit */
    kill(getppid(), SIGUSR1);
    exit(w_match_count % 256);
}
