#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "shared.h"
#include "utils.h"

/* Forward declarations for child process entry points */
void word_carrier_main(int wc_id, int floor_id);
void letter_carrier_main(int lc_id, int floor_id);
void sorting_main(int sort_id, int floor_id);
void delivery_elevator_main(void);
void reposition_elevator_main(void);

/* Table of all child PIDs for cleanup on exit */
static pid_t child_pids[MAX_CHILDREN];
static int   n_children = 0;

/* Command-line parameters */
static int  g_nf, g_wc_pf, g_lc_pf, g_s_pf, g_max_wf, g_dc, g_rc;
static char g_input[512]  = "input.txt";
static char g_output[512] = "output.txt";

static volatile int g_got_sigint = 0;

/* ── Signal handlers ── */

/* Reap zombie children without blocking */
static void handle_sigchld(int sig)
{
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

/* Set done flag so all processes exit cleanly on Ctrl+C */
static void handle_sigint(int sig)
{
    (void)sig;
    g_got_sigint = 1;
    if (shm) shm->done = 1;
}

/* Fork a child and record its PID */
static pid_t fork_child(void)
{
    fflush(stdout);
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); exit(EXIT_FAILURE); }
    if (pid > 0 && n_children < MAX_CHILDREN)
        child_pids[n_children++] = pid;
    return pid;
}

/* Register and fork a single letter-carrier on the given floor */
static int spawn_lc_on_floor(int floor_id)
{
    int lc_id = register_lc(floor_id, 0);
    if (lc_id < 0) { fprintf(stderr, "register_lc: out of slots\n"); return -1; }

    pid_t pid = fork_child();
    if (pid == 0) { letter_carrier_main(lc_id, floor_id); _exit(0); }
    shm->lc[lc_id].pid = pid;
    return lc_id;
}

/* Send SIGTERM to every child process */
static void kill_all_children(void)
{
    for (int i = 0; i < n_children; i++)
        if (child_pids[i] > 0) kill(child_pids[i], SIGTERM);
}

/* Write sorted output file: ordered by sorting_floor then word_id */
static void write_output(void)
{
    int idx[MAX_WORDS];
    int cnt = 0;
    for (int i = 0; i < shm->nw; i++) idx[cnt++] = i;

    /* Bubble sort indices by (sorting_floor, word_id) */
    for (int i = 0; i < cnt - 1; i++) {
        for (int j = i + 1; j < cnt; j++) {
            WordEntry *a = &shm->words[idx[i]];
            WordEntry *b = &shm->words[idx[j]];
            int sw = 0;
            if (a->sorting_floor > b->sorting_floor) sw = 1;
            else if (a->sorting_floor == b->sorting_floor && a->word_id > b->word_id) sw = 1;
            if (sw) { int t = idx[i]; idx[i] = idx[j]; idx[j] = t; }
        }
    }

    FILE *fp = fopen(g_output, "w");
    if (!fp) { perror("fopen(output)"); return; }

    for (int k = 0; k < cnt; k++) {
        WordEntry *w = &shm->words[idx[k]];
        char result[MAX_WORD_LEN] = {0};
        int  rlen = 0;
        /* Reconstruct word from sorting area */
        for (int i = 0; i < w->len; i++)
            if (w->occ[i]) result[rlen++] = w->area[i];
        if (rlen == 0) strncpy(result, w->orig, MAX_WORD_LEN - 1);
        fprintf(fp, "%d %s %d\n", w->word_id, result, w->sorting_floor);
    }
    fclose(fp);
}

static void usage(const char *prog)
{
    fprintf(stderr,
        "Usage: %s -f <floors> -w <wc/f> -l <lc/f> -s <sort/f>"
        " -c <cap> -d <deliv_cap> -r <repos_cap> -i <in> -o <out>\n",
        prog);
}

/* ══ main ══ */
int main(int argc, char *argv[])
{
    /* Unbuffered stdout so child output appears immediately */
    setvbuf(stdout, NULL, _IONBF, 0);

    int opt;
    int f_set=0,w_set=0,l_set=0,s_set=0,c_set=0,d_set=0,r_set=0;

    /* Parse command-line arguments with getopt */
    while ((opt = getopt(argc, argv, "f:w:l:s:c:d:r:i:o:")) != -1) {
        switch (opt) {
        case 'f': g_nf     = atoi(optarg); f_set=1; break;
        case 'w': g_wc_pf  = atoi(optarg); w_set=1; break;
        case 'l': g_lc_pf  = atoi(optarg); l_set=1; break;
        case 's': g_s_pf   = atoi(optarg); s_set=1; break;
        case 'c': g_max_wf = atoi(optarg); c_set=1; break;
        case 'd': g_dc     = atoi(optarg); d_set=1; break;
        case 'r': g_rc     = atoi(optarg); r_set=1; break;
        case 'i': strncpy(g_input,  optarg, 511); break;
        case 'o': strncpy(g_output, optarg, 511); break;
        default:  usage(argv[0]); return 1;
        }
    }

    /* Validate that all required flags were provided */
    if (!f_set||!w_set||!l_set||!s_set||!c_set||!d_set||!r_set) {
        usage(argv[0]); return 1;
    }
    if (g_nf < 1 || g_nf > MAX_FLOORS) {
        fprintf(stderr, "num_floors must be 1..%d\n", MAX_FLOORS); return 1;
    }
    if (g_wc_pf<1||g_lc_pf<1||g_s_pf<1||g_max_wf<1||g_dc<1||g_rc<1) {
        fprintf(stderr, "All numeric parameters must be >= 1\n"); return 1;
    }
    /* Verify input file is readable */
    FILE *chk = fopen(g_input, "r");
    if (!chk) { fprintf(stderr, "Cannot open: %s\n", g_input); return 1; }
    fclose(chk);

    /* Register signal handlers */
    struct sigaction sa = {0};
    sa.sa_handler = handle_sigchld; sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);
    sa.sa_handler = handle_sigint;  sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    printf("Program is starting...\n");
    printf("Input file is being read...\n");

    if (shm_init(g_nf, g_max_wf, g_dc, g_rc, g_lc_pf) < 0) return 1;
    printf("Shared memory is initialized...\n");

    int nw = load_words(g_input);
    if (nw <= 0) {
        fprintf(stderr, "No words loaded.\n"); shm_cleanup(); return 1;
    }

    printf("Synchronization primitives are created...\n");
    printf("Processes are being created...\n");
    printf("[PID:%d] Parent process started\n", (int)getpid());

    /* ── Spawn all child processes, one floor at a time ── */
    int wc_global   = 0;
    int sort_global = 0;

    for (int f = 0; f < g_nf; f++) {
        printf("--- Initializing Floor %d ---\n", f);

        /* Word-carrier processes */
        for (int w = 0; w < g_wc_pf; w++) {
            int id  = wc_global++;
            pid_t p = fork_child();
            if (p == 0) { word_carrier_main(id, f); _exit(0); }
            printf("[PID:%d] Word-carrier-process_%d initialized on floor %d\n",
                   (int)p, id, f);
        }

        /* Letter-carrier processes */
        for (int l = 0; l < g_lc_pf; l++) {
            int lc_id = spawn_lc_on_floor(f);
            pid_t p   = (lc_id >= 0) ? shm->lc[lc_id].pid : -1;
            printf("[PID:%d] Letter-carrier-process_%d initialized on floor %d\n",
                   (int)p, lc_id, f);
            SWAIT(&shm->floors[f].lc_mtx);
            shm->floors[f].lc_cnt++;
            SPOST(&shm->floors[f].lc_mtx);
        }

        /* Sorting processes */
        for (int s = 0; s < g_s_pf; s++) {
            int id  = sort_global++;
            pid_t p = fork_child();
            if (p == 0) { sorting_main(id, f); _exit(0); }
            printf("[PID:%d] Sorting-process_%d initialized on floor %d\n",
                   (int)p, id, f);
        }
    }

    /* Elevator processes (one of each) */
    {
        pid_t p = fork_child();
        if (p == 0) { delivery_elevator_main(); _exit(0); }
        shm->deliv.pid = p;
        printf("[PID:%d] Delivery elevator process started\n", (int)p);
    }
    {
        pid_t p = fork_child();
        if (p == 0) { reposition_elevator_main(); _exit(0); }
        shm->repos.pid = p;
        printf("[PID:%d] Reposition elevator process started\n", (int)p);
    }

    /* Release all children at the same time via the start barrier */
    printf("--------------------------------------------------\n");
    fflush(stdout);
    for (int i = 0; i < n_children; i++)
        SPOST(&shm->start_barrier);

    /* ── Monitor loop: wait for all words to complete ── */
    while (!g_got_sigint) {
        if (shm->n_done >= shm->nw) break;

        /* Respawn letter-carriers on any floor that has none left */
        for (int f = 0; f < g_nf; f++) {
            if (shm->floors[f].lc_cnt == 0 && !shm->done) {
                for (int _r = 0; _r < g_lc_pf; _r++) {
                    int lc_id = spawn_lc_on_floor(f);
                    if (lc_id >= 0) {
                        printf("[PID:%d] Letter-carrier-process_%d initialized on floor %d"
                               " (respawn)\n",
                               (int)shm->lc[lc_id].pid, lc_id, f);
                        SWAIT(&shm->floors[f].lc_mtx);
                        shm->floors[f].lc_cnt++;
                        SPOST(&shm->floors[f].lc_mtx);
                        SPOST(&shm->start_barrier);
                    }
                }
            }
        }
        sleep_ms(100);
    }

    /* ── Shutdown: wake blocked children then kill them ── */
    shm->done = 1;
    SPOST(&shm->deliv.avail);
    SPOST(&shm->repos.avail);
    for (int f = 0; f < g_nf; f++) {
        SPOST(&shm->floors[f].lc_work);
        SPOST(&shm->floors[f].sort_work);
    }
    for (int i = 0; i < shm->nlc; i++)
        SPOST(&shm->lc[i].done);

    sleep_ms(500);
    kill_all_children();
    sleep_ms(200);
    for (int i = 0; i < n_children; i++)
        if (child_pids[i] > 0) waitpid(child_pids[i], NULL, 0);

    /* ── Write output file and print summary ── */
    printf("--------------------------------------------------\n");
    printf("All words have been transported and sorted...\n");
    printf("Output file is being created...\n");
    write_output();

    printf("\nSystem Summary:\n");
    printf("Total words: %d\n",                  shm->nw);
    printf("Completed words: %d\n",              shm->n_done);
    printf("Retries: %d\n",                      shm->stats.retries);
    printf("Characters transported: %d\n",       shm->stats.chars_xported);
    printf("Delivery elevator operations: %d\n", shm->deliv.ops);
    printf("Reposition elevator operations: %d\n", shm->repos.ops);
    printf("Program terminated successfully.\n");

    shm_cleanup();
    return 0;
}
