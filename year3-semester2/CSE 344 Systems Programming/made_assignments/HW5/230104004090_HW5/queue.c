#include "queue.h"
#include <stdlib.h>
#include <string.h>

void pq_init(PriorityQueue *pq) {
    pq->head = NULL;
    pq->size = 0;
}

/*
 * Insert in sorted position:
 *   - Lower priority value = higher urgency (EXPRESS=1 first)
 *   - Tie-break: lower id goes first
 */
void pq_push(PriorityQueue *pq, Order order) {
    QNode *node = malloc(sizeof(QNode));
    if (!node) return;
    node->order = order;
    node->next  = NULL;

    /* Find insertion point */
    QNode **cur = &pq->head;
    while (*cur) {
        int higher_prio = (order.priority < (*cur)->order.priority);
        int same_prio_lower_id = (order.priority == (*cur)->order.priority &&
                                  order.id < (*cur)->order.id);
        if (higher_prio || same_prio_lower_id) break;
        cur = &(*cur)->next;
    }
    node->next = *cur;
    *cur = node;
    pq->size++;
}

int pq_pop(PriorityQueue *pq, Order *out) {
    if (!pq->head) return 0;
    QNode *tmp = pq->head;
    *out = tmp->order;
    pq->head = tmp->next;
    free(tmp);
    pq->size--;
    return 1;
}

size_t pq_size(const PriorityQueue *pq) {
    return pq->size;
}

void pq_destroy(PriorityQueue *pq) {
    QNode *cur = pq->head;
    while (cur) {
        QNode *next = cur->next;
        free(cur);
        cur = next;
    }
    pq->head = NULL;
    pq->size = 0;
}
