#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "shared.h"
#include "utils.h"

/* Place a delivered character into the word's sorting area */
static void place_char(WordEntry *w, int task_idx, int lc_id, int dest_floor)
{
    int pid = (int)getpid();

    SWAIT(&w->sort_lock);

    /* Find first unoccupied, non-fixed slot */
    int slot = -1;
    for (int i = 0; i < w->len; i++) {
        if (!w->occ[i] && !w->fix[i]) { slot = i; break; }
    }
    /* Fallback: accept any unoccupied slot */
    if (slot < 0) {
        for (int i = 0; i < w->len; i++) {
            if (!w->occ[i]) { slot = i; break; }
        }
    }

    char ch = w->tasks[task_idx].ch;

    if (slot >= 0) {
        w->area[slot] = ch;
        w->occ[slot]  = 1;
    }

    w->tasks[task_idx].delivered = 1;
    w->delivered_cnt++;
    int all_deliv = (w->delivered_cnt == w->num_tasks);

    SPOST(&w->sort_lock);

    printf("[PID:%d] Letter-carrier-process_%d brought char '%c' of word %d to floor %d\n",
           pid, lc_id, ch, w->word_id, dest_floor);

    /* Wake a sorting process on the destination floor */
    SPOST(&shm->floors[dest_floor].sort_work);

    /* Release arrival floor capacity once all chars are delivered */
    if (all_deliv && w->arrival_floor != w->sorting_floor) {
        SWAIT(&shm->adm_mtx);
        if (!w->arr_dec_done) {
            w->arr_dec_done = 1;
            shm->floors[w->arrival_floor].word_count--;
        }
        SPOST(&shm->adm_mtx);
    }
}

/* Add an elevator request and signal the elevator to wake up */
static void request_elevator(ElevState *elev, int lc_id, int from, int to)
{
    int slot = -1;

    /* Loop until a free request slot is available (no recursion) */
    while (slot < 0) {
        SWAIT(&elev->mtx);
        for (int i = 0; i < MAX_ELEV_REQ; i++) {
            if (!elev->req[i].active) { slot = i; break; }
        }
        if (slot < 0) {
            /* Queue full: release lock and wait before retrying */
            SPOST(&elev->mtx);
            sleep_ms(10);
        }
    }

    elev->req[slot].active     = 1;
    elev->req[slot].lc_id      = lc_id;
    elev->req[slot].from_floor = from;
    elev->req[slot].to_floor   = to;
    elev->req_n++;

    SPOST(&elev->mtx);
    SPOST(&elev->avail);  /* wake the elevator process */
}

/* Main loop for a letter-carrier process */
void letter_carrier_main(int lc_id, int floor_id)
{
    SWAIT(&shm->start_barrier);

    shm->lc[lc_id].pid = getpid();
    int pid = (int)getpid();

    int cur = floor_id;
    shm->lc[lc_id].cur_floor = cur;

    while (!shm->done) {

        /* ── Scan current floor for an unclaimed task (random pick) ── */
        int found_word = -1;
        int found_task = -1;

        SWAIT(&shm->floors[cur].task_mtx);
        {
            /* Collect words with at least one unclaimed task on this floor */
            int cands[MAX_WORDS];
            int ncands = 0;
            for (int wi = 0; wi < shm->nw; wi++) {
                WordEntry *w = &shm->words[wi];
                if (!w->admitted || w->completed) continue;
                if (w->arrival_floor != cur) continue;
                for (int ti = 0; ti < w->num_tasks; ti++) {
                    if (!w->tasks[ti].claimed && !w->tasks[ti].delivered)
                        { cands[ncands++] = wi; break; }
                }
            }
            if (ncands > 0) {
                /* Pick a random word and then a random task from that word */
                struct timespec rts;
                clock_gettime(CLOCK_MONOTONIC, &rts);
                unsigned seed = (unsigned)(rts.tv_nsec ^ ((unsigned)lc_id * 999983u));
                int pick_w = cands[seed % (unsigned)ncands];
                WordEntry *pw = &shm->words[pick_w];

                int tcands[MAX_WORD_LEN];
                int ntcands = 0;
                for (int ti = 0; ti < pw->num_tasks; ti++) {
                    if (!pw->tasks[ti].claimed && !pw->tasks[ti].delivered)
                        tcands[ntcands++] = ti;
                }
                if (ntcands > 0) {
                    seed ^= ((unsigned)(lc_id + 7) * 1000033u);
                    int pick_t = tcands[seed % (unsigned)ntcands];
                    pw->tasks[pick_t].claimed    = 1;  /* claim atomically under task_mtx */
                    pw->tasks[pick_t].carrier_id = lc_id;
                    found_word = pick_w;
                    found_task = pick_t;
                }
            }
        }
        SPOST(&shm->floors[cur].task_mtx);

        if (found_task >= 0) {
            WordEntry *w  = &shm->words[found_word];
            CharTask  *t  = &w->tasks[found_task];
            int dest      = t->dest_floor;
            char ch       = t->ch;

            printf("[PID:%d] Letter-carrier-process_%d selected char '%c' of word %d"
                   " from floor %d\n",
                   pid, lc_id, ch, w->word_id, cur);

            if (dest == cur) {
                /* Same floor: place directly without using the elevator */
                printf("[PID:%d] Letter-carrier-process_%d requested delivery elevator"
                       " from floor %d to floor %d\n",
                       pid, lc_id, cur, dest);
                printf("[PID:%d] Destination is same floor → direct placement\n", pid);
                place_char(w, found_task, lc_id, dest);
            } else {
                /* Different floor: ride the delivery elevator */
                shm->lc[lc_id].pending_to    = dest;
                shm->lc[lc_id].carry_ch      = ch;
                shm->lc[lc_id].carry_word_id  = w->word_id;

                printf("[PID:%d] Destination is same floor? No\n", pid);
                printf("[PID:%d] Letter-carrier-process_%d requested delivery elevator"
                       " from floor %d to floor %d\n",
                       pid, lc_id, cur, dest);

                SWAIT(&shm->floors[cur].lc_mtx);
                shm->floors[cur].lc_cnt--;  /* leaving current floor */
                SPOST(&shm->floors[cur].lc_mtx);

                request_elevator(&shm->deliv, lc_id, cur, dest);
                SWAIT(&shm->lc[lc_id].done);  /* block until elevator drops us off */

                if (shm->done) break;

                cur = dest;
                shm->lc[lc_id].cur_floor = cur;
                shm->lc[lc_id].carry_ch  = 0;

                SWAIT(&shm->floors[cur].lc_mtx);
                shm->floors[cur].lc_cnt++;  /* arrived at new floor */
                SPOST(&shm->floors[cur].lc_mtx);

                place_char(w, found_task, lc_id, dest);
            }

            /* Update global character transport counter */
            SWAIT(&shm->stats.mtx);
            shm->stats.chars_xported++;
            SPOST(&shm->stats.mtx);

        } else {
            /* No task found: reposition to a random different floor */
            printf("[PID:%d] Letter-carrier-process_%d found no available task"
                   " on floor %d\n",
                   pid, lc_id, cur);

            int rand_floor = cur;
            if (shm->nf > 1) {
                struct timespec ts;
                clock_gettime(CLOCK_MONOTONIC, &ts);
                srand((unsigned)(ts.tv_nsec ^ ((unsigned)lc_id * 1000003u)));
                do { rand_floor = rand() % shm->nf; } while (rand_floor == cur);
            }

            shm->lc[lc_id].pending_to   = rand_floor;
            shm->lc[lc_id].carry_ch     = 0;
            shm->lc[lc_id].carry_word_id = -1;

            SWAIT(&shm->floors[cur].lc_mtx);
            shm->floors[cur].lc_cnt--;  /* leaving current floor */
            SPOST(&shm->floors[cur].lc_mtx);

            request_elevator(&shm->repos, lc_id, cur, rand_floor);

            printf("[PID:%d] Letter-carrier-process_%d requested reposition elevator"
                   " from floor %d\n",
                   pid, lc_id, cur);

            SWAIT(&shm->lc[lc_id].done);  /* block until reposition elevator drops us off */
            if (shm->done) break;

            cur = rand_floor;
            shm->lc[lc_id].cur_floor = cur;

            SWAIT(&shm->floors[cur].lc_mtx);
            shm->floors[cur].lc_cnt++;  /* arrived at new floor */
            SPOST(&shm->floors[cur].lc_mtx);

            printf("[PID:%d] Letter-carrier-process_%d resumed work on floor %d\n",
                   pid, lc_id, cur);

            sem_twait(&shm->floors[cur].lc_work, 50);  /* wait briefly for work to appear */
        }
    }
}
