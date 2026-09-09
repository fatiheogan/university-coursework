#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "shared.h"
#include "utils.h"

/* Main loop for a word-carrier process */
void word_carrier_main(int wc_id, int floor_id)
{
    SWAIT(&shm->start_barrier);

    int pid = (int)getpid();

    while (!shm->done) {

        /* ── Pick the next unclaimed word in round-robin order ── */
        int widx = -1;

        SWAIT(&shm->rr_mtx);
        int start = shm->rr;
        for (int i = 0; i < shm->nw; i++) {
            int idx = (start + i) % shm->nw;
            WordEntry *w = &shm->words[idx];
            if (!w->claimed && !w->admitted && !w->completed) {
                w->claimed = 1;                   /* claim atomically under rr_mtx */
                shm->rr    = (idx + 1) % shm->nw; /* advance round-robin cursor */
                widx       = idx;
                break;
            }
        }
        SPOST(&shm->rr_mtx);

        if (widx < 0) {
            if (shm->n_done >= shm->nw) break;  /* all words done, exit */
            sleep_ms(20);
            continue;
        }

        WordEntry *w = &shm->words[widx];

        printf("[PID:%d] Word-carrier-process_%d claimed word %d\n",
               pid, wc_id, w->word_id);

        /* ── All-or-nothing floor capacity check ── */
        int arr      = floor_id;
        int srt      = w->sorting_floor;
        int admitted = 0;

        /* Validate sorting floor is within system bounds */
        if (srt < 0 || srt >= shm->nf) {
            fprintf(stderr, "[PID:%d] Word %d has invalid sorting_floor %d (max %d), skipping\n",
                    pid, w->word_id, srt, shm->nf - 1);
            SWAIT(&shm->rr_mtx);
            w->claimed = 0;
            SPOST(&shm->rr_mtx);
            continue;
        }

        SWAIT(&shm->adm_mtx);
        if (arr == srt) {
            /* Arrival and sorting floor are the same: reserve one slot */
            if (shm->floors[arr].word_count + 1 <= shm->max_wf) {
                shm->floors[arr].word_count++;
                admitted = 1;
            }
        } else {
            /* Both floors must have capacity; reserve atomically */
            if (shm->floors[arr].word_count + 1 <= shm->max_wf &&
                shm->floors[srt].word_count + 1 <= shm->max_wf) {
                shm->floors[arr].word_count++;
                shm->floors[srt].word_count++;
                admitted = 1;
            }
        }
        if (admitted) {
            w->arrival_floor = arr;
            w->admitted      = 1;
        }
        SPOST(&shm->adm_mtx);

        if (admitted) {
            printf("[PID:%d] Word %d admitted to floor %d (sorting floor: %d)\n",
                   pid, w->word_id, arr, srt);
            /* Wake one letter-carrier per character task */
            for (int i = 0; i < w->num_tasks; i++)
                SPOST(&shm->floors[arr].lc_work);
        } else {
            /* Admission failed: release the word so another carrier can retry */
            SWAIT(&shm->rr_mtx);
            w->claimed = 0;
            SPOST(&shm->rr_mtx);

            SWAIT(&shm->stats.mtx);
            shm->stats.retries++;
            SPOST(&shm->stats.mtx);

            sleep_ms(10);
        }
    }
}
