#include <linux/module.h>
#include <linux/kernel.h>

/* Device specific files*/
#include <linux/genhd.h>
#include <linux/blkdev.h>

#include <linux/workqueue.h>

#include "queue.h"
#include "ldd.h"
#include "proc_entries.h"

static int major_num = 0;
static int logical_blk_size = 512;
static int nsectors = 1024;

/* Threshold for IO's to be cached */
int threshold_io_count = 0;
module_param(threshold_io_count, int, 0);

static struct kmem_cache *cache = NULL;
static struct workqueue_struct *wq;
static struct ldd_device ldd_dev;
static queue *q;
struct driver_stats dev_stat;


/**
 * Dequeues cached IO's and serves  the request
 */
static void wq_function(struct work_struct *work)
{       
        struct io_entry *io = NULL;
        printk (KERN_INFO "ldd: Flush IO\n");
        while(1)
        {
                if (queue_isempty(q))
                        break;
                
                printk(KERN_INFO "ldd: serving cached IO\n");
                io = (struct io_entry *)queue_dequeue(q);
                spin_lock(&(ldd_dev.lock));
                memcpy(ldd_dev.data + io->offset, io->buffer, io->nbytes);
                spin_unlock(&(ldd_dev.lock));
         	kmem_cache_free(cache, io);
                spin_lock(&(dev_stat.lock));
                dev_stat.driver_memory -= sizeof(struct io_entry);
                spin_unlock(&(dev_stat.lock));        
        }
        kfree(work);
        spin_lock(&(dev_stat.lock));
        dev_stat.driver_memory -= sizeof(struct work_struct);
        spin_unlock(&(dev_stat.lock));

        return;
}

/**
 * On request from proc entry it dequeues and serves the request. 
 */
void flush_io(void)
{       
        struct io_entry *io = NULL;
        printk (KERN_INFO "ldd: Flushing pending IO\n");
        while(1)
        {
                if (queue_isempty(q))
                        break;
                
                printk(KERN_INFO "ldd: serving cached IO\n");
                io = (struct io_entry *)queue_dequeue(q);
                spin_lock(&(ldd_dev.lock));
                memcpy(ldd_dev.data + io->offset, io->buffer, io->nbytes);
                spin_unlock(&(ldd_dev.lock));
         	kmem_cache_free(cache, io);
                spin_lock(&(dev_stat.lock));
                dev_stat.driver_memory -= sizeof(struct io_entry);
                spin_unlock(&(dev_stat.lock));        
        }
        return;
}

/**
 * Handle an I/O request.
 */
static void ldd_transfer(struct ldd_device *dev, sector_t sector,
                         unsigned long nsect, char *buffer, int write) {
        struct work_struct *work = NULL;
        struct io_entry *io = NULL;
        
        if (write) {

                if (wq) {                        
                        io = kmem_cache_alloc(cache, GFP_KERNEL);
                        spin_lock(&(dev_stat.lock));
                        dev_stat.driver_memory += sizeof(struct io_entry);
                        spin_unlock(&(dev_stat.lock));

                        io->nbytes = nsect * logical_blk_size;
                        io->offset = sector * logical_blk_size;
                        memcpy(io->buffer, buffer, io->nbytes);
                        queue_enqueue(q, (void*)io);
                        printk(KERN_INFO "ldd: cached IO request\n");
                        if (queue_isfull(q)) { 

                         /* Allocate memory for struct work */
                            work = (struct work_struct *)
                                    kmalloc(sizeof(struct work_struct), GFP_KERNEL);
                            spin_lock(&(dev_stat.lock));
                            dev_stat.driver_memory += sizeof(struct work_struct);
                            spin_unlock(&(dev_stat.lock));

                            if (work) {
				    INIT_WORK(work, wq_function);
				    queue_work(wq, work);
                            }                   
                        }
                }
        }
}

/**
 * Handles request from request queue. 
 */
static void ldd_request(struct request_queue *q) {
        struct request *req;

        req = blk_fetch_request(q);
        while (req != NULL) {
                if (req == NULL || (req->cmd_type != REQ_TYPE_FS)) {
                        printk (KERN_NOTICE "ldd: Skip non-CMD request\n");
                        __blk_end_request_all(req, -EIO);
                        continue;
                }
                ldd_transfer(&ldd_dev, blk_rq_pos(req),
			     blk_rq_cur_sectors(req), req->buffer,
			     rq_data_dir(req));
                if ( ! __blk_end_request_cur(req, 0) ) {
                        req = blk_fetch_request(q);
                }
        }
}

/**
 * The device operations structure.
 */
static struct block_device_operations ldd_ops = {
                .owner  = THIS_MODULE
};

/**
 * Constructor for struct io_entry
 */
static void io_entry_constructor(void *buffer)
{
	struct io_entry *io = (struct io_entry *)buffer;
	io->nbytes = 0;
	io->offset = 0;
}

ssize_t total_in_memory_data(void)
{
        dev_stat.total_in_memory = (queue_entries(q) * 
				    sizeof(struct io_entry));
        return dev_stat.total_in_memory;
}

static int __init ldd_init(void) {

        dev_stat.driver_memory = 0;
        dev_stat.total_in_memory = 0;
        dev_stat.batches_flushed = 0;
        spin_lock_init(&dev_stat.lock);

        q = queue_create();	
        cache = kmem_cache_create("mem_cache", sizeof(struct io_entry),
				  0,
				  (SLAB_RECLAIM_ACCOUNT|
                                   SLAB_PANIC|SLAB_MEM_SPREAD|
				   SLAB_NOLEAKTRACE),
				  io_entry_constructor);
        /*
         * Set up our internal device.
         */
        ldd_dev.size = nsectors * logical_blk_size;
        spin_lock_init(&ldd_dev.lock);
        ldd_dev.data = vmalloc(ldd_dev.size);
        if (ldd_dev.data == NULL)
                return -ENOMEM;
        spin_lock(&(dev_stat.lock));
        dev_stat.driver_memory += sizeof(ldd_dev.size);
        spin_unlock(&(dev_stat.lock));
        
        /*
         * Get a request queue.
         */
        ldd_dev.req_queue = blk_init_queue(ldd_request, &ldd_dev.lock);
        if (ldd_dev.req_queue == NULL)
                goto out;
        blk_queue_logical_block_size(ldd_dev.req_queue, logical_blk_size);
        /*
         * Get registered.
         */
        major_num = register_blkdev(major_num, "sdc");
        if (major_num < 0) {
                printk(KERN_WARNING "sdc: unable to get major number\n");
                goto out;
        }
        /*
         * And the gendisk structure.
         */
        ldd_dev.gd = alloc_disk(16);
        if (!ldd_dev.gd)
                goto out_unregister;
        ldd_dev.gd->major = major_num;
        ldd_dev.gd->first_minor = 0;
        ldd_dev.gd->fops = &ldd_ops;
        ldd_dev.gd->private_data = &ldd_dev;
        strcpy(ldd_dev.gd->disk_name, "sdc0");
        set_capacity(ldd_dev.gd, nsectors);
        ldd_dev.gd->queue = ldd_dev.req_queue;
        add_disk(ldd_dev.gd);
        printk (KERN_INFO "ldd: ldd init successful");
        
        /* create workqueue */
        wq = create_workqueue("my_queue");

        /* create proc entries */
        create_proc_entries();
        return 0;

out_unregister:
        unregister_blkdev(major_num, "sdc");
out:
        vfree(ldd_dev.data);
        spin_lock(&(dev_stat.lock));
        dev_stat.driver_memory -= sizeof(ldd_dev.size);
        spin_unlock(&(dev_stat.lock));

        return -ENOMEM;
}

static void __exit ldd_exit(void)
{
        flush_io();
	kmem_cache_destroy(cache);
        del_gendisk(ldd_dev.gd);
        put_disk(ldd_dev.gd);
        unregister_blkdev(major_num, "sdc");
        blk_cleanup_queue(ldd_dev.req_queue);
        vfree(ldd_dev.data);
        flush_workqueue(wq);
        destroy_workqueue(wq);
        queue_delete(q);
        remove_proc_entries();
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ashay Shirwadkar");

module_init(ldd_init);
module_exit(ldd_exit);
