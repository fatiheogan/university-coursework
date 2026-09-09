#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "dirutils.h"
#include "pattern.h"


/* Returns a list of immediate subdirectories under root */
char **get_subdirs(const char *root, int *count)
{
    *count = 0;
    DIR *d = opendir(root);
    if (!d) return NULL;

    /* Allocate array to hold subdirectory paths */
    char **list = malloc(MAX_SUBDIRS * sizeof(char *));
    if (!list) { closedir(d); return NULL; }

    struct dirent *e;
    struct stat    st;
    char           fp[MAX_PATH];

    /* Iterate entries keep only directories */
    while ((e = readdir(d)) != NULL && *count < MAX_SUBDIRS) {
        if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
        snprintf(fp, sizeof(fp), "%s/%s", root, e->d_name);
        if (stat(fp, &st) == 0 && S_ISDIR(st.st_mode))
            list[(*count)++] = strdup(fp);
    }
    closedir(d);
    return list;
}


/* Recursively searches dir for files matching pattern and size filters */
void direct_search(const char  *dir,
                   const char  *pattern,
                   long         min_size,
                   MatchResult *results,
                   int         *result_cnt,
                   int         *total_scan)
{
    DIR *d = opendir(dir);
    if (!d) return;

    struct dirent *e;
    struct stat    st;
    char           fp[MAX_PATH * 2];

    while ((e = readdir(d)) != NULL) {
        if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
        snprintf(fp, sizeof(fp), "%s/%s", dir, e->d_name);

        if (lstat(fp, &st) < 0) continue;

        if (S_ISDIR(st.st_mode)) {
            /* Recurse into subdirectory */
            direct_search(fp, pattern, min_size, results, result_cnt, total_scan);
        } else if (S_ISREG(st.st_mode)) {
            (*total_scan)++;
            /* Skip file if it's smaller than min_size */
            if (min_size > 0 && st.st_size < min_size) continue;
            /* Check pattern match and store result if matched */
            if (pattern_match(pattern, e->d_name) && *result_cnt < MAX_MATCHES) {
                strncpy(results[*result_cnt].path, fp, MAX_PATH - 1);
                results[*result_cnt].path[MAX_PATH - 1] = '\0';
                results[*result_cnt].size       = st.st_size;
                results[*result_cnt].worker_pid = (pid_t)getpid();
                (*result_cnt)++;
                /* Print match immediately to stdout */
                printf("[Parent PID:%d] MATCH: %s (%ld bytes)\n",
                       (int)getpid(), fp, (long)st.st_size);
                fflush(stdout);
            }
        }
    }
    closedir(d);
}
