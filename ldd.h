#ifndef _LDD_H_
#define _LDD_H_

/*
 * We can tweak our hardware sector size, but the kernel talks to us
 * in terms of small sectors, always.
 */
#define KERNEL_SECTOR_SIZE 512
#define MAX_BUFSIZE 4096
/*
 * The internal representation of our device.
 */
struct ldd_device {
        unsigned long size;
        spinlock_t lock;
        u8 *data;
        struct gendisk *gd;
        struct request_queue *req_queue;
};

struct io_entry {
        unsigned long offset;
        unsigned long nbytes;
        char buffer[MAX_BUFSIZE]; //FIXME
};

struct driver_stats {
        ssize_t driver_memory;
        ssize_t total_in_memory;
        int batches_flushed;
        spinlock_t lock;
};

ssize_t total_in_memory_data(void);
void flush_io(void);
#endif
