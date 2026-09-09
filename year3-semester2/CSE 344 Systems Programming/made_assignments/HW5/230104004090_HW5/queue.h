#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>

#define MAX_NAME_LEN 33

typedef enum {
    EXPRESS  = 1,
    STANDARD = 2,
    ECONOMY  = 3
} Priority;

typedef struct {
    int      id;
    char     name[MAX_NAME_LEN];
    Priority priority;
    int      duration;  /* simulation units, 1 unit = 500 ms */
} Order;

typedef struct QNode {
    Order        order;
    struct QNode *next;
} QNode;

typedef struct {
    QNode  *head;
    size_t  size;
} PriorityQueue;

void  pq_init(PriorityQueue *pq);
void  pq_push(PriorityQueue *pq, Order order);
int   pq_pop(PriorityQueue *pq, Order *out);   /* returns 1 on success, 0 if empty */
size_t pq_size(const PriorityQueue *pq);
void  pq_destroy(PriorityQueue *pq);

#endif /* QUEUE_H */
