#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <linux/spinlock.h>
#include <linux/slab.h>

typedef struct queue_entry
{
        struct list_head    list;      // Linked List
        void                *item;     // Item of Data
} queue_entry;

typedef struct queue
{
        struct list_head    list;      // List Head
        int       no_of_elements;      // Total number of elements
        spinlock_t          lock;      // Lock
} queue;

queue   *queue_create(void);
void     queue_delete(queue *q);
void     queue_enqueue(queue *q, void *item);
void    *queue_dequeue(queue *q);
int      queue_isempty(queue *q);
int      queue_lock(queue *q);
void     queue_unlock(queue *q);
int      queue_isfull(queue *q);
ssize_t  queue_entries(queue *q);

#endif /* _QUEUE_H_ */
