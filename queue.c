#include <linux/module.h>
#include <linux/list.h>

#include "queue.h"
#include "ldd.h"

extern int threshold_io_count;
extern struct driver_stats dev_stat;

queue *queue_create(void)
{
        queue *q;
	
        q = (queue *)kmalloc(sizeof(queue), GFP_KERNEL);
        if (!q) {
                printk(KERN_ALERT "queue_create:kmalloc failed\n");
                return q;
        }
        
        INIT_LIST_HEAD(&(q->list));
        spin_lock_init(&(q->lock));
        q->no_of_elements = 0;
        printk(KERN_INFO "queue created\n");
        
        return q;
}

// queue must be empty!
void queue_delete(queue *q)
{
        kfree((void *) q);
        return;
}

ssize_t queue_entries(queue *q)
{
    return q->no_of_elements;
}

void queue_enqueue(queue *q, void *item)
{
        queue_entry *entry;
        
        entry = (queue_entry *) kmalloc(sizeof(queue_entry), GFP_KERNEL);
        if (! entry) {
                printk(KERN_ALERT "queue_enqueue:kmalloc failed\n");
                return;
        }
        
        entry->item = item;
        printk(KERN_INFO "enqueued element %d\n", threshold_io_count);
        queue_lock(q);
        list_add_tail(&(entry->list), &(q->list));
        q->no_of_elements++;
        queue_unlock(q);
        
        return;
}

void *queue_dequeue(queue *q)
{
        queue_entry *entry;
        void *item;
        
        if (queue_isempty(q)) {
                return NULL;
        }
        
        queue_lock(q);
        entry = list_first_entry(&(q->list), queue_entry, list);
        item = entry->item;
        list_del(&(entry->list));
        kfree((void *) entry);
        q->no_of_elements--;
        queue_unlock(q);
        dev_stat.batches_flushed++;
        printk(KERN_INFO "dequeued element\n");
        return item;
}

int queue_isempty(queue *q)
{
        int ret;
        
        queue_lock(q);
        ret = list_empty(&(q->list));
        queue_unlock(q);
        
        return ret;
}

int queue_isfull(queue *q)
{
        int ret = 0;
        
        queue_lock(q);
        if (q->no_of_elements == threshold_io_count)
           ret = 1;
        queue_unlock(q);
        
        return ret;
}

int queue_lock(queue *q)
{
        spin_lock(&(q->lock));
        return 0;
}

void queue_unlock(queue *q)
{
        spin_unlock(&(q->lock));
        return;
}
/*
MODULE_LICENSE("GPL");
static int __init queue_init(void) {
        queue *q;
        void  *item;
        int ret = 0;
        
        q = queue_create();
        
        item = kmalloc(20, GFP_KERNEL);
        queue_enqueue(q, item);
        
        item = kmalloc(20, GFP_KERNEL);
        queue_enqueue(q, item);
        
        item = kmalloc(20, GFP_KERNEL);
        ret = queue_isfull(q);
        if (ret)
            goto out1;
        queue_enqueue(q, item);
        
        while(1)
        {
                if (queue_isempty(q))
                        break;
                
                item = queue_dequeue(q);
                kfree(item);
        }
        
        queue_delete(q);        
        goto out;
 out1:
        printk(KERN_INFO "queue full\n");
 out:
        return ret;
}

static void __exit queue_exit(void)
{
        return;
}

module_init(queue_init);
module_exit(queue_exit);

*/
