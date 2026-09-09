#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "shared.h"
#include "utils.h"

/* Pointers to the shared memory segment, accessible by all processes */
SharedMemory *shm      = NULL;
char          shm_name[64] = {0};

/* Create and initialise the shared memory segment */
int shm_init(int nf, int max_wf, int dc, int rc, int lc_pf)
{
    /* Use PID in name to avoid collisions with other runs */
    snprintf(shm_name, sizeof(shm_name), "/hw3_shm_%d", (int)getpid());

    int fd = shm_open(shm_name, O_CREAT | O_RDWR | O_EXCL, 0600);
    if (fd < 0) { perror("shm_open"); return -1; }

    size_t sz = sizeof(SharedMemory);
    if (ftruncate(fd, (off_t)sz) < 0) {
        perror("ftruncate"); close(fd); return -1;
    }

    shm = mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (shm == MAP_FAILED) { perror("mmap"); return -1; }

    memset(shm, 0, sz);

    /* Store system-wide configuration */
    shm->nf        = nf;
    shm->max_wf    = max_wf;
    shm->deliv_cap = dc;
    shm->repos_cap = rc;
    shm->lc_pf     = lc_pf;

    /* Global semaphores */
    SEM_INIT(&shm->rr_mtx,        1);  /* round-robin word scanner */
    SEM_INIT(&shm->lc_reg_mtx,    1);  /* LC slot allocator */
    SEM_INIT(&shm->adm_mtx,       1);  /* atomic floor capacity check */
    SEM_INIT(&shm->comp_mtx,      1);  /* n_done counter */
    SEM_INIT(&shm->start_barrier, 0);  /* holds children until parent is ready */
    SEM_INIT(&shm->stats.mtx,     1);  /* statistics counters */

    /* Per-floor semaphores and counters */
    for (int f = 0; f < nf; f++) {
        SEM_INIT(&shm->floors[f].floor_mtx, 1);  /* word_count */
        SEM_INIT(&shm->floors[f].task_mtx,  1);  /* char-task claiming */
        SEM_INIT(&shm->floors[f].lc_work,   0);  /* new task arrived signal */
        SEM_INIT(&shm->floors[f].sort_work, 0);  /* char placed signal */
        SEM_INIT(&shm->floors[f].lc_mtx,    1);  /* lc_cnt */
        shm->floors[f].word_count = 0;
        shm->floors[f].lc_cnt    = 0;
    }

    /* Delivery elevator initial state */
    SEM_INIT(&shm->deliv.mtx,   1);
    SEM_INIT(&shm->deliv.avail, 0);
    shm->deliv.nf    = nf;
    shm->deliv.cur   = 0;
    shm->deliv.dir   = 0;
    shm->deliv.cap   = dc;
    shm->deliv.load_n = 0;
    shm->deliv.req_n  = 0;

    /* Reposition elevator initial state */
    SEM_INIT(&shm->repos.mtx,   1);
    SEM_INIT(&shm->repos.avail, 0);
    shm->repos.nf    = nf;
    shm->repos.cur   = 0;
    shm->repos.dir   = 0;
    shm->repos.cap   = rc;
    shm->repos.load_n = 0;
    shm->repos.req_n  = 0;

    return 0;
}

/* Unmap and remove the shared memory segment */
void shm_cleanup(void)
{
    if (shm && shm != MAP_FAILED) {
        munmap(shm, sizeof(SharedMemory));
        shm = NULL;
    }
    if (shm_name[0])
        shm_unlink(shm_name);
}

/* Parse input file and populate shm->words[].
   Each line format: word_id  word_string  sorting_floor */
int load_words(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (!fp) { perror("fopen(input)"); return -1; }

    int n = 0;
    char buf[MAX_WORD_LEN];
    int  wid, sfloor;

    while (n < MAX_WORDS && fscanf(fp, "%d %63s %d", &wid, buf, &sfloor) == 3) {
        WordEntry *w = &shm->words[n];
        memset(w, 0, sizeof(*w));

        w->word_id       = wid;
        strncpy(w->orig, buf, MAX_WORD_LEN - 1);
        w->len           = (int)strlen(w->orig);
        w->sorting_floor = sfloor;
        w->arrival_floor = -1;   /* filled in when a word-carrier admits the word */
        w->claimed       = 0;
        w->admitted      = 0;
        w->completed     = 0;
        w->arr_dec_done  = 0;
        w->delivered_cnt = 0;
        w->num_tasks     = w->len;

        /* One CharTask per character */
        for (int i = 0; i < w->len; i++) {
            CharTask *t  = &w->tasks[i];
            t->word_idx  = n;
            t->word_id   = wid;
            t->ch        = w->orig[i];
            t->orig_idx  = i;
            t->dest_floor = sfloor;
            t->claimed   = 0;
            t->delivered = 0;
            t->carrier_id = -1;
        }

        memset(w->area, 0, sizeof(w->area));
        memset(w->occ,  0, sizeof(w->occ));
        memset(w->fix,  0, sizeof(w->fix));

        /* One sorter/placer at a time per word */
        SEM_INIT(&w->sort_lock, 1);

        n++;
    }

    fclose(fp);
    shm->nw = n;
    return n;
}

/* Allocate a new LCState slot and return its id (thread-safe) */
int register_lc(int floor_id, pid_t pid)
{
    SWAIT(&shm->lc_reg_mtx);

    if (shm->nlc >= MAX_LCS) {
        SPOST(&shm->lc_reg_mtx);
        return -1;
    }

    int id = shm->nlc++;
    LCState *lc = &shm->lc[id];
    lc->id         = id;
    lc->pid        = pid;
    lc->init_floor = floor_id;
    lc->cur_floor  = floor_id;
    lc->active     = 1;
    lc->pending_to = -1;
    lc->carry_ch   = 0;
    lc->carry_word_id = -1;
    SEM_INIT(&lc->done, 0);  /* elevator posts this when LC is dropped off */

    SPOST(&shm->lc_reg_mtx);
    return id;
}
