#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "shared.h"
#include "utils.h"

/* Shared loop for both elevators, runs until system is done */
static void elevator_loop(const char *name, ElevState *elev)
{
    int pid = (int)getpid();
    int prev_floor = elev->cur;  /* start at actual floor to avoid false "arrived" print */

    while (!shm->done) {

        SWAIT(&elev->mtx);  /* lock elevator state */

        int cur = elev->cur;
        int nf  = elev->nf;

        /* Print arrival message only if floor changed and there is cargo/requests */
        if (cur != prev_floor && (elev->load_n > 0 || elev->req_n > 0))
            printf("[PID:%d] %s arrived at floor %d\n", pid, name, cur);
        prev_floor = cur;

        /* ── 1. Drop off passengers whose destination is cur floor ── */
        int drop_ids[MAX_ELEV_LOAD];
        int ndrop = 0;
        for (int i = elev->load_n - 1; i >= 0; i--) {
            if (elev->load_to[i] == cur) {
                drop_ids[ndrop++] = elev->load_lc[i];
                /* Remove by overwriting with last element */
                elev->load_lc[i] = elev->load_lc[elev->load_n - 1];
                elev->load_to[i] = elev->load_to[elev->load_n - 1];
                elev->load_n--;
                elev->ops++;
            }
        }
        if (ndrop > 0) {
            printf("[PID:%d] %s drop off at floor %d (currently %d letter-carrier inside):\n",
                   pid, name, cur, elev->load_n);
            for (int k = 0; k < ndrop; k++) {
                int lc_id = drop_ids[k];
                LCState *lc = &shm->lc[lc_id];
                printf(" Letter-carrier-process_%d", lc_id);
                if (lc->carry_ch)
                    printf(" carrying char '%c' of word %d", lc->carry_ch, lc->carry_word_id);
                printf("\n");
                SPOST(&lc->done);  /* wake the dropped-off carrier */
            }
        }

        /* ── 2. Pick up all waiting carriers at cur floor ── */
        int pick_ids[MAX_ELEV_LOAD];
        int pick_tos[MAX_ELEV_LOAD];
        int npick = 0;
        for (int i = 0; i < MAX_ELEV_REQ && elev->load_n + npick < elev->cap; i++) {
            ElevReq *req = &elev->req[i];
            if (!req->active || req->from_floor != cur) continue;
            pick_ids[npick] = req->lc_id;
            pick_tos[npick] = req->to_floor;
            npick++;
            req->active = 0;  /* free the request slot */
            elev->req_n--;
            elev->ops++;
        }
        if (npick > 0) {
            for (int k = 0; k < npick; k++) {
                elev->load_lc[elev->load_n] = pick_ids[k];
                elev->load_to[elev->load_n] = pick_tos[k];
                elev->load_n++;
            }
            printf("[PID:%d] %s pick up (currently %d letter-carrier inside):\n",
                   pid, name, elev->load_n);
            for (int k = 0; k < npick; k++) {
                int lc_id = pick_ids[k];
                LCState *lc = &shm->lc[lc_id];
                printf(" Letter-carrier-process_%d", lc_id);
                if (lc->carry_ch)
                    printf(" carrying char '%c' of word %d", lc->carry_ch, lc->carry_word_id);
                printf("\n");
            }
        }

        /* ── 3. Decide direction (LOOK algorithm) ── */
        int has_above = 0, has_below = 0;
        for (int i = 0; i < elev->load_n; i++) {
            if (elev->load_to[i] > cur) has_above = 1;
            if (elev->load_to[i] < cur) has_below = 1;
        }
        for (int i = 0; i < MAX_ELEV_REQ; i++) {
            if (!elev->req[i].active) continue;
            if (elev->req[i].from_floor > cur || elev->req[i].to_floor > cur) has_above = 1;
            if (elev->req[i].from_floor < cur || elev->req[i].to_floor < cur) has_below = 1;
        }

        /* If no work at all, wait for a new request without busy-looping */
        int nothing = (elev->load_n == 0 && elev->req_n == 0);
        if (nothing) {
            elev->dir = 0;
            SPOST(&elev->mtx);
            sem_twait(&elev->avail, 100);
            continue;
        }

        /* Pick initial direction if currently stopped */
        if (elev->dir == 0) {
            if (has_above)      elev->dir = 1;
            else if (has_below) elev->dir = -1;
        }

        /* Reverse at top/bottom or when no more work in current direction */
        if (elev->dir == 1 && (cur >= nf - 1 || !has_above))
            elev->dir = has_below ? -1 : 0;
        else if (elev->dir == -1 && (cur <= 0 || !has_below))
            elev->dir = has_above ? 1 : 0;

        /* Move one floor */
        if (elev->dir != 0) {
            elev->cur = cur + elev->dir;
            printf("[PID:%d] %s moving %s\n",
                   pid, name, elev->dir > 0 ? "UP" : "DOWN");
        }

        SPOST(&elev->mtx);
        sleep_ms(5);  /* simulate travel time */
    }
}

/* Delivery elevator: transports letter-carriers with characters */
void delivery_elevator_main(void)
{
    SWAIT(&shm->start_barrier);
    elevator_loop("Delivery elevator", &shm->deliv);
}

/* Reposition elevator: moves idle letter-carriers to other floors */
void reposition_elevator_main(void)
{
    SWAIT(&shm->start_barrier);
    elevator_loop("Reposition elevator", &shm->repos);
}
