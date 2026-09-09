#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "shared.h"
#include "utils.h"

/* One fix/swap pass over a word's sorting area.
   Must be called while holding word->sort_lock.
   Returns 1 if the word is fully sorted, 0 otherwise. */
static int do_sort_pass(WordEntry *w, int sort_id, int floor_id)
{
    int  pid = (int)getpid();
    int  len = w->len;
    int  any_active = 0;

    /* Skip the word if there are no occupied, non-fixed slots to work on */
    for (int i = 0; i < len; i++) {
        if (w->occ[i] && !w->fix[i]) { any_active = 1; break; }
    }
    if (!any_active) {
        for (int i = 0; i < len; i++) if (!w->fix[i]) return 0;
        return 1;
    }

    printf("[PID:%d] Sorting-process_%d is scanning word %d on floor %d\n",
           pid, sort_id, w->word_id, floor_id);

    for (int i = 0; i < len; i++) {
        if (!w->occ[i]) continue;  /* empty slot, skip */
        if (w->fix[i])  continue;  /* already fixed, skip */

        /* First time seeing this slot: print detected + placed messages */
        if (!w->logged[i]) {
            w->logged[i] = 1;
            printf("[PID:%d] Sorting-process_%d detected char '%c' of word %d"
                   " on floor %d\n",
                   pid, sort_id, w->area[i], w->word_id, floor_id);
            printf("[PID:%d] Sorting-process_%d placed char '%c' into sorting area\n",
                   pid, sort_id, w->area[i]);
        }

        /* Case: char is already in its correct position → fix it */
        if (w->area[i] == w->orig[i]) {
            w->fix[i] = 1;
            printf("[PID:%d] Sorting-process_%d determined correct position"
                   " for char '%c'\n",
                   pid, sort_id, w->area[i]);
            printf("[PID:%d] Sorting-process_%d fixed char '%c' of word %d\n",
                   pid, sort_id, w->area[i], w->word_id);
            continue;
        }

        /* Find the target slot where this char belongs */
        int target = -1;
        for (int j = 0; j < len; j++) {
            if (j == i || w->fix[j]) continue;
            if (w->orig[j] == w->area[i]) { target = j; break; }
        }
        if (target < 0) continue;  /* no valid target found */

        printf("[PID:%d] Sorting-process_%d found char '%c' misplaced\n",
               pid, sort_id, w->area[i]);

        if (!w->occ[target]) {
            /* Target is empty: move char directly */
            char ch         = w->area[i];
            w->area[target] = ch;
            w->occ[target]  = 1;
            w->area[i]      = 0;
            w->occ[i]       = 0;
            w->logged[target] = 1;

            printf("[PID:%d] Sorting-process_%d moved char '%c' to correct index\n",
                   pid, sort_id, ch);

            if (w->area[target] == w->orig[target]) {
                w->fix[target] = 1;
                printf("[PID:%d] Sorting-process_%d fixed char '%c' of word %d\n",
                       pid, sort_id, w->area[target], w->word_id);
            }

        } else if (!w->fix[target]) {
            /* Target is occupied and not fixed: swap the two chars */
            char tmp        = w->area[i];
            w->area[i]      = w->area[target];
            w->area[target] = tmp;

            printf("[PID:%d] Sorting-process_%d swapped char '%c' with another non-fixed char\n",
                   pid, sort_id, w->area[target]);

            /* Check if either slot is now correctly placed after the swap */
            if (w->area[i] == w->orig[i]) {
                w->fix[i] = 1;
                printf("[PID:%d] Sorting-process_%d fixed one more position of word %d\n",
                       pid, sort_id, w->word_id);
            }
            if (w->area[target] == w->orig[target]) {
                w->fix[target] = 1;
                printf("[PID:%d] Sorting-process_%d fixed one more position of word %d\n",
                       pid, sort_id, w->word_id);
            }
        }
        /* If target is fixed, do nothing (assignment rule) */
    }

    /* Word is complete only when every position is fixed */
    for (int i = 0; i < len; i++)
        if (!w->fix[i]) return 0;
    return 1;
}

/* Main loop for a sorting process */
void sorting_main(int sort_id, int floor_id)
{
    SWAIT(&shm->start_barrier);

    int pid = (int)getpid();

    while (!shm->done) {
        int found = 0;

        for (int wi = 0; wi < shm->nw; wi++) {
            WordEntry *w = &shm->words[wi];

            if (!w->admitted || w->completed)  continue;
            if (w->sorting_floor != floor_id)  continue;
            if (w->delivered_cnt == 0)          continue;  /* nothing to sort yet */

            /* Try to acquire the per-word lock without blocking */
            if (sem_trywait(&w->sort_lock) != 0) continue;

            int just_completed = 0;
            if (!w->completed) {
                int done = do_sort_pass(w, sort_id, floor_id);
                if (done) {
                    w->completed   = 1;
                    just_completed = 1;
                    printf("[PID:%d] Word %d COMPLETED\n", pid, w->word_id);
                }
            }

            sem_post(&w->sort_lock);

            if (just_completed) {
                /* Release the sorting floor's capacity slot */
                SWAIT(&shm->adm_mtx);
                shm->floors[floor_id].word_count--;
                if (w->arrival_floor == floor_id)
                    w->arr_dec_done = 1;
                SPOST(&shm->adm_mtx);

                /* Increment global completion counter, signal parent if all done */
                SWAIT(&shm->comp_mtx);
                shm->n_done++;
                SPOST(&shm->comp_mtx);
            }

            found = 1;
        }

        /* No work found: wait briefly for a new char to arrive */
        if (!found)
            sem_twait(&shm->floors[floor_id].sort_work, 50);
    }
}
