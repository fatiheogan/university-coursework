#ifndef SHARED_H
#define SHARED_H

#include <semaphore.h>
#include <sys/types.h>

/* ── Compile-time sizing constants ── */
#define MAX_WORD_LEN   64
#define MAX_WORDS      500
#define MAX_FLOORS     100
#define MAX_LCS        3000    /* total letter-carrier processes ever created */
#define MAX_ELEV_REQ   10000   /* elevator request queue length               */
#define MAX_ELEV_LOAD  500     /* max chars/carriers inside one elevator      */
#define MAX_CHILDREN   20000   /* max child PIDs tracked by parent            */

/* One entry per character of a word; claimed and delivered by letter-carriers */
typedef struct {
    int  word_idx;          /* index into shm->words[]               */
    int  word_id;
    char ch;
    int  orig_idx;          /* position in the original word         */
    int  dest_floor;        /* equals sorting_floor of the word      */
    volatile int claimed;   /* 0 = free, 1 = taken by an LC          */
    volatile int delivered; /* 1 = placed in sorting area            */
    int  carrier_id;        /* which LC claimed it (-1 = none)       */
} CharTask;

/* Full state of one word from admission to completion */
typedef struct {
    int  word_id;
    char orig[MAX_WORD_LEN];  /* original word string                */
    int  len;
    int  sorting_floor;
    int  arrival_floor;       /* set when word-carrier admits word   */

    /* Sorting buffer: characters placed here by letter-carriers */
    char area[MAX_WORD_LEN];  /* '\0' = empty slot                   */
    int  occ[MAX_WORD_LEN];   /* 1 = slot occupied                   */
    int  fix[MAX_WORD_LEN];   /* 1 = position permanently fixed      */
    int  logged[MAX_WORD_LEN];/* 1 = sorter already detected it      */

    /* Life-cycle flags */
    volatile int claimed;     /* a word-carrier is holding this word */
    volatile int admitted;    /* word is active in the system        */
    volatile int completed;   /* all fix[] == 1                      */
    volatile int arr_dec_done;/* arrival-floor count already decremented */

    /* Character task list */
    CharTask tasks[MAX_WORD_LEN];
    int      num_tasks;
    volatile int delivered_cnt; /* how many chars reached sorting area */

    /* Protects sorting area; one writer (sorter or LC) at a time */
    sem_t sort_lock;
} WordEntry;

/* Single slot in an elevator's request queue */
typedef struct {
    int  active;
    int  lc_id;
    int  from_floor;
    int  to_floor;
} ElevReq;

/* Full state of one elevator (shared between elevator process and LCs) */
typedef struct {
    int  nf;          /* copy of system num_floors                   */
    int  cur;         /* current floor                               */
    int  dir;         /* 1=UP  -1=DOWN  0=idle                       */
    int  cap;         /* max simultaneous passengers                 */

    /* On-board passengers */
    int  load_lc[MAX_ELEV_LOAD];  /* LC ids on board                 */
    int  load_to[MAX_ELEV_LOAD];  /* their destination floors        */
    int  load_n;                  /* number on board                 */

    /* Pending requests (sparse array, scan for active slots) */
    ElevReq req[MAX_ELEV_REQ];
    int     req_n;    /* number of active requests                   */

    sem_t mtx;        /* protects all fields above                   */
    sem_t avail;      /* posted when a new request is added          */

    volatile int ops; /* total pick-up + drop-off events             */
    pid_t pid;
} ElevState;

/* Per-floor counters and synchronisation primitives */
typedef struct {
    volatile int word_count; /* active words on this floor            */
    sem_t  floor_mtx;        /* protects word_count                   */
    sem_t  task_mtx;         /* protects char-task scanning/claiming  */
    sem_t  lc_work;          /* posted when new tasks appear here     */
    sem_t  sort_work;        /* posted when a char is placed here     */

    volatile int lc_cnt;     /* number of LCs currently on this floor */
    sem_t  lc_mtx;           /* protects lc_cnt                       */
} FloorData;

/* State of one letter-carrier, visible to parent and elevators */
typedef struct {
    pid_t pid;
    int   id;
    int   init_floor;
    volatile int cur_floor;
    volatile int active;

    /* Set before requesting an elevator ride */
    volatile int pending_to;   /* destination floor                  */
    char  carry_ch;            /* char being carried (delivery only) */
    int   carry_word_id;       /* word id of that char               */

    /* Elevator posts this semaphore when LC arrives at destination */
    sem_t done;
} LCState;

/* Global counters protected by a single mutex */
typedef struct {
    volatile int retries;
    volatile int chars_xported;
    sem_t mtx;
} Stats;

/* Master shared-memory segment; mapped into every process */
typedef struct {
    /* System configuration (written by parent before forking) */
    int nf;            /* num_floors                                */
    int max_wf;        /* max active words per floor                */
    int deliv_cap;     /* delivery elevator capacity                */
    int repos_cap;     /* reposition elevator capacity              */
    int lc_pf;         /* letter-carriers per floor (for respawn)   */

    /* Word table */
    WordEntry words[MAX_WORDS];
    int nw;            /* total words loaded                        */

    /* Round-robin word selector */
    volatile int rr;   /* next index to scan                        */
    sem_t rr_mtx;

    /* Floors */
    FloorData floors[MAX_FLOORS];

    /* Elevators */
    ElevState deliv;
    ElevState repos;

    /* Letter-carrier registry */
    LCState lc[MAX_LCS];
    volatile int nlc;  /* total LC slots allocated so far           */
    sem_t lc_reg_mtx;

    /* Termination */
    volatile int done;        /* 1 = system is shutting down        */
    volatile int n_done;      /* number of words completed          */
    sem_t comp_mtx;           /* protects n_done                    */

    /* Atomic floor-capacity reservation during admission */
    sem_t adm_mtx;

    /* Children wait here until parent finishes printing init messages */
    sem_t start_barrier;

    /* Statistics */
    Stats stats;

} SharedMemory;

/* Exported globals */
extern SharedMemory *shm;
extern char          shm_name[64];

/* API */
int  shm_init(int nf, int max_wf, int dc, int rc, int lc_pf);
void shm_cleanup(void);
int  load_words(const char *path);
int  register_lc(int floor_id, pid_t pid);

#endif /* SHARED_H */
