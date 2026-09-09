#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "output.h"

/* Global arrays to store all match results and scan count */
MatchResult g_results    [MAX_MATCHES];
int         g_result_cnt = 0;
int         g_total_scan = 0;

/* Reads each worker's temp file and fills g_results */
void collect_results(int nw, char tmp_paths[][64], pid_t *pids)
{
    for (int w = 0; w < nw; w++) {
        FILE *f = fopen(tmp_paths[w], "r");
        if (!f) continue;

        pid_t wpid = (pids ? pids[w] : 0);
        char  line [MAX_PATH * 2 + 64];

        while (fgets(line, sizeof(line), f)) {
            line[strcspn(line, "\r\n")] = '\0';

            if (line[0] == '#') {
                /* Header lines: #PID or #SCANNED */
                if (strncmp(line, "#PID ", 5) == 0)
                    wpid = (pid_t)atoi(line + 5);
                else if (strncmp(line, "#SCANNED ", 9) == 0)
                    g_total_scan += atoi(line + 9);
                continue;
            }

            /* Each data line is: path\tsize */
            char *tab = strchr(line, '\t');
            if (!tab || g_result_cnt >= MAX_MATCHES) continue;

            *tab = '\0';
            strncpy(g_results[g_result_cnt].path, line, MAX_PATH - 1);
            g_results[g_result_cnt].path[MAX_PATH - 1] = '\0';
            g_results[g_result_cnt].size       = atol(tab + 1);
            g_results[g_result_cnt].worker_pid = wpid;
            g_result_cnt++;
        }
        fclose(f);
        /* Remove temp file after reading */
        unlink(tmp_paths[w]);
    }
}

/* Comparator for sorting results by path alphabetically */
static int cmp_results(const void *a, const void *b)
{
    return strcmp(((const MatchResult *)a)->path,
                  ((const MatchResult *)b)->path);
}

/* Builds the "|--" indent prefix based on tree depth */
static void make_prefix(char *buf, size_t bufsz, int depth)
{
    if (bufsz < 2) return;
    buf[0] = '|';
    size_t pos = 1;

    if (depth == 1) {
        /* depth 1 -> "|-- " */
        if (pos + 3 < bufsz) { memcpy(buf + pos, "-- ", 3); pos += 3; }
    } else {
        /* deeper levels get extra 6 dashes per level */
        if (pos + 2 < bufsz) { memcpy(buf + pos, "--", 2); pos += 2; }
        for (int i = 0; i < depth - 1 && pos + 6 < bufsz; i++) {
            memcpy(buf + pos, "------", 6);
            pos += 6;
        }
        if (pos + 1 < bufsz) buf[pos++] = ' ';
    }
    buf[pos] = '\0';
}

/* Prints all matches as an indented directory tree */
void print_tree(const char *root)
{
    if (g_result_cnt == 0) {
        printf("No matching files found.\n");
        return;
    }

    /* Sort results so the tree is printed in order */
    qsort(g_results, g_result_cnt, sizeof(MatchResult), cmp_results);

    printf("%s\n", root);

    int  root_len = (int)strlen(root);

    /* Keep track of directories already printed to avoid duplicates */
    static char printed  [MAX_PRINTED][MAX_PATH];
    int         n_printed = 0;

    for (int i = 0; i < g_result_cnt; i++) {
        /* Get path relative to root */
        const char *rel = g_results[i].path + root_len;
        if (*rel == '/') rel++;

        /* Tokenize relative path into components */
        char  tmp[MAX_PATH];
        strncpy(tmp, rel, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = '\0';

        char *comps[64];
        int   nc = 0;
        for (char *tok = strtok(tmp, "/"); tok && nc < 64; tok = strtok(NULL, "/"))
            comps[nc++] = tok;

        if (nc == 0) continue;

        /* Print parent directories that haven't been printed yet */
        char cur[MAX_PATH];
        strncpy(cur, root, sizeof(cur) - 1);
        cur[sizeof(cur) - 1] = '\0';

        for (int c = 0; c < nc - 1; c++) {
            char dir_fp[MAX_PATH * 2];
            snprintf(dir_fp, sizeof(dir_fp), "%s/%s", cur, comps[c]);

            int already = 0;
            for (int p = 0; p < n_printed; p++) {
                if (!strcmp(printed[p], dir_fp)) { already = 1; break; }
            }
            if (!already && n_printed < MAX_PRINTED) {
                char pfx[256];
                make_prefix(pfx, sizeof(pfx), c + 1);
                printf("%s%s\n", pfx, comps[c]);
                strncpy(printed[n_printed++], dir_fp, MAX_PATH - 1);
            }
            strncpy(cur, dir_fp, sizeof(cur) - 1);
        }

        /* Print the matched file with size and worker PID */
        char pfx[256];
        make_prefix(pfx, sizeof(pfx), nc);
        printf("%s%s (%ld bytes) [Worker %d]\n",
               pfx, comps[nc - 1],
               g_results[i].size,
               (int)g_results[i].worker_pid);
    }
}

/* Prints the final summary: worker count, files scanned, matches per worker */
void print_summary(int nw, pid_t *worker_pids)
{
    printf("--- Summary ---\n");
    printf("Total workers used : %d\n", nw);
    printf("Total files scanned : %d\n", g_total_scan);
    printf("Total matches found : %d\n", g_result_cnt);

    for (int i = 0; i < nw; i++) {
        /* Count how many matches belong to this worker */
        int cnt = 0;
        for (int r = 0; r < g_result_cnt; r++)
            if (g_results[r].worker_pid == worker_pids[i]) cnt++;

        printf("Worker PID %d : %d %s\n",
               (int)worker_pids[i],
               cnt,
               cnt == 1 ? "match" : "matches");
    }
}
