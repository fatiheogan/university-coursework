#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#include "shm.h"
#include "reader.h"
#include "dispatcher.h"
#include "analyzer.h"
#include "aggregator.h"
#include "watchdog.h"

/* ---- Global shutdown flag (SIGINT) ---- */
static volatile sig_atomic_t shutdown_watchdog = 0;
static volatile sig_atomic_t sigint_received   = 0;

/* All child PIDs (parent tracks) */
#define MAX_CHILDREN 128
static pid_t child_pids[MAX_CHILDREN];
static int   num_children = 0;

static void sigint_handler(int sig) {
    (void)sig;
    sigint_received = 1;
}

/* ---- Parse keywords from comma-separated string ---- */
static int parse_keywords(const char *kw_str, config_t *cfg) {
    char buf[512];
    strncpy(buf, kw_str, sizeof(buf) - 1);
    buf[sizeof(buf)-1] = '\0';
    char *tok = strtok(buf, ",");
    while (tok && cfg->num_keywords < MAX_KEYWORDS) {
        strncpy(cfg->keywords[cfg->num_keywords], tok, 63);
        cfg->keywords[cfg->num_keywords][63] = '\0';
        cfg->num_keywords++;
        tok = strtok(NULL, ",");
    }
    return cfg->num_keywords;
}

/* ---- Read config file (one log file per line) ---- */
static int read_config_file(const char *path, config_t *cfg) {
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "Cannot open config file: %s\n", path);
        return -1;
    }
    char line[256];
    while (fgets(line, sizeof(line), f) && cfg->num_files < MAX_FILES) {
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[--len] = '\0';
        if (len == 0) continue;
        strncpy(cfg->log_files[cfg->num_files], line, 255);
        cfg->log_files[cfg->num_files][255] = '\0';
        cfg->num_files++;
    }
    fclose(f);
    return cfg->num_files;
}

/* ---- Print usage ---- */
static void usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s -c <config> -f <filter> -k <keywords> -t <reader_threads>\n"
        "       -w <worker_threads> -a <cap_A> -b <cap_B> -d <cap_D>\n"
        "       [-T <timeout>] -o <output> -O <binary_output>\n",
        prog);
}

/* ---- Parse command line ---- */
static int parse_args(int argc, char *argv[], config_t *cfg) {
    memset(cfg, 0, sizeof(*cfg));
    cfg->timeout_sec = 10;  /* default */

    int opt;
    while ((opt = getopt(argc, argv, "c:f:k:t:w:a:b:d:T:o:O:")) != -1) {
        switch (opt) {
        case 'c': read_config_file(optarg, cfg); break;
        case 'f': strncpy(cfg->filter_file, optarg, 255); break;
        case 'k': parse_keywords(optarg, cfg); break;
        case 't': cfg->num_reader_threads = atoi(optarg); break;
        case 'w': cfg->num_worker_threads = atoi(optarg); break;
        case 'a': cfg->capacity_a = atoi(optarg); break;
        case 'b': cfg->capacity_b = atoi(optarg); break;
        case 'd': cfg->capacity_d = atoi(optarg); break;
        case 'T': cfg->timeout_sec = atoi(optarg); break;
        case 'o': strncpy(cfg->output_file, optarg, 255); break;
        case 'O': strncpy(cfg->binary_file, optarg, 255); break;
        default:  usage(argv[0]); return -1;
        }
    }

    /* Validate */
    if (cfg->num_files < 1) { fprintf(stderr, "Error: no log files in config\n"); return -1; }
    if (cfg->num_keywords < 1) { fprintf(stderr, "Error: -k required\n"); return -1; }
    if (cfg->num_reader_threads < 1) { fprintf(stderr, "Error: -t >= 1\n"); return -1; }
    if (cfg->num_worker_threads < 1 || cfg->num_worker_threads > MAX_WORKERS) {
        fprintf(stderr, "Error: -w in [1, %d]\n", MAX_WORKERS); return -1; }
    if (cfg->capacity_a < 4)  { fprintf(stderr, "Error: -a >= 4\n"); return -1; }
    if (cfg->capacity_b < 4)  { fprintf(stderr, "Error: -b >= 4\n"); return -1; }
    if (cfg->capacity_d < 2)  { fprintf(stderr, "Error: -d >= 2\n"); return -1; }
    if (cfg->timeout_sec < 1) { fprintf(stderr, "Error: -T >= 1\n"); return -1; }
    if (cfg->output_file[0] == '\0') { fprintf(stderr, "Error: -o required\n"); return -1; }
    if (cfg->binary_file[0] == '\0') { fprintf(stderr, "Error: -O required\n"); return -1; }
    if (cfg->filter_file[0] == '\0') { fprintf(stderr, "Error: -f required\n"); return -1; }

    return 0;
}

/* ---- Register a child PID ---- */
static void register_child(pid_t pid) {
    if (num_children < MAX_CHILDREN)
        child_pids[num_children++] = pid;
}

int main(int argc, char *argv[]) {
    config_t cfg;
    if (parse_args(argc, argv, &cfg) != 0)
        return 1;

    printf("[PID:%d] Parent started. Files: %d, Keywords: ",
           (int)getpid(), cfg.num_files);
    for (int k = 0; k < cfg.num_keywords; k++) {
        if (k > 0) printf(",");
        printf("%s", cfg.keywords[k]);
    }
    printf("\n");
    fflush(stdout);

    /* ---- Initialize shared memory ---- */
    shm_handle_t shm;
    if (shm_init(&shm, &cfg) != 0) {
        fprintf(stderr, "Failed to initialize shared memory\n");
        return 1;
    }
    printf("[PID:%d] Shared memory initialized (A:%d B:%dx4 D:%d).\n",
           (int)getpid(), cfg.capacity_a, cfg.capacity_b, cfg.capacity_d);
    fflush(stdout);

    /* ---- Create pipe pairs for heartbeats (one per reader) ---- */
    int pipe_read[MAX_FILES], pipe_write[MAX_FILES];
    for (int i = 0; i < cfg.num_files; i++) {
        int pipefd[2];
        if (pipe(pipefd) != 0) { perror("pipe"); return 1; }
        /* Set non-blocking read end */
        int flags = fcntl(pipefd[0], F_GETFL, 0);
        fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);
        pipe_read[i]  = pipefd[0];
        pipe_write[i] = pipefd[1];
    }

    /* ---- Install SIGINT handler ---- */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);

    /* ---- Fork Reader Processes ---- */
    for (int i = 0; i < cfg.num_files; i++) {
        printf("[PID:%d] Forking Reader %d -> %s\n",
               (int)getpid(), i, cfg.log_files[i]);
        fflush(stdout);

        pid_t pid = fork();
        if (pid < 0) { perror("fork reader"); return 1; }
        if (pid == 0) {
            /* Child: close all pipe_read ends and other write ends */
            for (int j = 0; j < cfg.num_files; j++) {
                close(pipe_read[j]);
                if (j != i) close(pipe_write[j]);
            }
            reader_process_main(i, cfg.log_files[i],
                                 cfg.num_reader_threads,
                                 pipe_write[i],
                                 &shm, &cfg);
            _exit(0); /* should not reach */
        }
        register_child(pid);
        close(pipe_write[i]); /* parent closes write end */
    }

    /* ---- Fork Dispatcher Process ---- */
    printf("[PID:%d] Forking Dispatcher\n", (int)getpid());
    fflush(stdout);
    {
        pid_t pid = fork();
        if (pid < 0) { perror("fork dispatcher"); return 1; }
        if (pid == 0) {
            for (int i = 0; i < cfg.num_files; i++) close(pipe_read[i]);
            dispatcher_process_main(&shm, &cfg);
            _exit(0);
        }
        register_child(pid);
    }

    /* ---- Fork Analyzer Processes ---- */
    const char *lvl_names[] = {"ERROR","WARN","INFO","DEBUG"};
    for (int lvl = 0; lvl < NUM_LEVELS; lvl++) {
        printf("[PID:%d] Forking Analyzer %s (index %d)\n",
               (int)getpid(), lvl_names[lvl], lvl);
        fflush(stdout);

        pid_t pid = fork();
        if (pid < 0) { perror("fork analyzer"); return 1; }
        if (pid == 0) {
            for (int i = 0; i < cfg.num_files; i++) close(pipe_read[i]);
            analyzer_process_main(lvl, &shm, &cfg);
            _exit(0);
        }
        register_child(pid);
    }

    /* ---- Fork Aggregator Process ---- */
    printf("[PID:%d] Forking Aggregator\n", (int)getpid());
    fflush(stdout);
    {
        pid_t pid = fork();
        if (pid < 0) { perror("fork aggregator"); return 1; }
        if (pid == 0) {
            for (int i = 0; i < cfg.num_files; i++) close(pipe_read[i]);
            aggregator_process_main(&shm, &cfg);
            _exit(0);
        }
        register_child(pid);
    }

    /* ---- Start Watchdog Thread ---- */
    watchdog_arg_t warg;
    memset(&warg, 0, sizeof(warg));
    for (int i = 0; i < cfg.num_files; i++) {
        warg.read_fds[i] = pipe_read[i];
        strncpy(warg.file_names[i], cfg.log_files[i], 255);
    }
    warg.num_readers    = cfg.num_files;
    warg.num_children   = num_children;
    warg.shutdown_flag  = &shutdown_watchdog;

    pthread_t watchdog_tid;
    if (pthread_create(&watchdog_tid, NULL, watchdog_thread_func, &warg) != 0) {
        perror("pthread_create watchdog");
        return 1;
    }
    printf("[PID:%d] Watchdog thread started.\n", (int)getpid());
    fflush(stdout);

    /* ---- Wait for all children ---- */
    int children_remaining = num_children;
    while (children_remaining > 0) {
        if (sigint_received) {
            /* Send SIGTERM to all children */
            for (int i = 0; i < num_children; i++)
                kill(child_pids[i], SIGTERM);

            /* Wait up to 5 seconds */
            time_t deadline = time(NULL) + 5;
            while (children_remaining > 0 && time(NULL) < deadline) {
                int status;
                pid_t dead = waitpid(-1, &status, WNOHANG);
                if (dead > 0) children_remaining--;
                else usleep(100000);
            }
            break;
        }

        int status;
        pid_t dead = waitpid(-1, &status, 0);
        if (dead > 0) {
            children_remaining--;
            warg.num_children = children_remaining;
        } else if (dead < 0 && errno == ECHILD) {
            break;
        }
    }

    /* ---- Stop watchdog ---- */
    shutdown_watchdog = 1;
    pthread_join(watchdog_tid, NULL);

    /* ---- Read final results from Region C ---- */
    region_c_t *rc = shm.region_c;
    long total_entries = 0;
    double total_weighted = 0.0;
    double high_priority = shm.region_d->high_priority_score;
    for (int i = 0; i < NUM_LEVELS; i++) {
        total_entries   += rc->results[i].total_entries;
        total_weighted  += rc->results[i].total_weighted_score;
    }
    /* Print final summary */
    printf("==================================================\n");
    printf("SYSTEM SUMMARY\n");
    printf("Keywords   : ");
    for (int k = 0; k < cfg.num_keywords; k++) {
        if (k > 0) printf(", ");
        printf("%s", cfg.keywords[k]);
    }
    printf("\n");
    printf("Log files  : %d\n", cfg.num_files);
    printf("Total entries : %ld\n", total_entries);
    printf("Total weighted : %.1f\n", total_weighted);
    printf("High-priority : %.1f (source filter: %s)\n",
           high_priority, cfg.filter_file);
    for (int i = 0; i < NUM_LEVELS; i++) {
        printf("  %s : %ld entries, score: %.1f\n",
               rc->results[i].level,
               rc->results[i].total_entries,
               rc->results[i].total_weighted_score);
    }
    printf("==================================================\n");
    printf("Program terminated successfully.\n");
    fflush(stdout);

    /* ---- Cleanup shared memory ---- */
    shm_destroy(&shm);

    /* Close remaining pipe read ends */
    for (int i = 0; i < cfg.num_files; i++)
        if (pipe_read[i] >= 0) close(pipe_read[i]);

    if (sigint_received) _exit(1);
    return 0;
}
