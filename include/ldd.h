#ifndef _LDD_H_
#define _LDD_H_

#define KERNEL_SECTOR_SIZE 512
#define MAX_BUFSIZE 4096

/*
 * The internal representation of our device.
 */
struct ldd_device {
        unsigned long size;              // size of device
        spinlock_t lock;                 // lock for updating elements in structure
        u8 *data;                        // write/read data from device
        struct gendisk *gd;              // gendisk structure
        struct request_queue *req_queue; // request_queue
};

struct io_entry {
        unsigned long offset;            // offset from which data is to be written
        unsigned long nbytes;            // size of data
        char buffer[MAX_BUFSIZE];        // data pointer
};

struct driver_stats {
        ssize_t driver_memory;           // total memory taken by driver
        ssize_t total_in_memory;         // total in memory data
        int batches_flushed;             // batches of IO's flushed
        spinlock_t lock;                 // lock to update fields inside stuct
};

ssize_t total_in_memory_data(void);
void flush_io(void);

#endif /* _LDD_H_ */
