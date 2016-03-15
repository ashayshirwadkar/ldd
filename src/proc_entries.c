#include <linux/proc_fs.h>

#include "proc_entries.h"
#include "ldd.h"

struct proc_dir_entry *proc;
static char proc_priv_data[4][8] = {"proc_1", "proc_2", "proc_3", "proc_4"};
static char op_buf[MAXLEN];

extern int total_batches;
extern struct driver_stats dev_stat;

/**
 * Read proc entries
 */
ssize_t proc_read(struct file *filp, char *buf, size_t count, loff_t *offp) 
{
        char *data = NULL;
        int data_len = 0;
        ssize_t size = 0;

        data = PDE_DATA(file_inode(filp));
        if(!(data)){
                printk(KERN_INFO "Null data");
                return 0;
        }
        
        /* proc_1: Displays total amount of memory used by driver */
        if (!strncmp(data, proc_priv_data[0], strlen(proc_priv_data[0]))) {
                spin_lock(&(dev_stat.lock));
                sprintf(op_buf, "Total memory taken by driver: %ldB",
                        dev_stat.driver_memory);
                spin_unlock(&(dev_stat.lock));
                data_len = strlen(op_buf);
                if(count > data_len) {
                        count = data_len;
                }
                count = simple_read_from_buffer(buf, count, offp, op_buf,
                                                data_len);
        } 
        /* proc_2: Total batches of data flushed */
        else if (!strncmp(data, proc_priv_data[1], strlen(proc_priv_data[1]))) {
                sprintf(op_buf, "Batches of IO's fulshed: %d",
                        dev_stat.batches_flushed);
                data_len = strlen(op_buf);
                if(count > data_len) {
                        count = data_len;
                }
                count = simple_read_from_buffer(buf, count, offp, op_buf,
                                                data_len);
        }
        /* proc_4: Total in-memory data */
        else if (!strncmp(data, proc_priv_data[3], strlen(proc_priv_data[3]))) {
                size = total_in_memory_data();
                sprintf(op_buf, "Total in-memory data : %ldB",(long) size);
                data_len = strlen(op_buf);
                if(count > data_len) {
                        count = data_len;
                }
                count = simple_read_from_buffer(buf, count, offp, op_buf,
                                                data_len);
        }
        else {
                /*Do nothing*/
                count = -EINVAL;
        }
        return count;
}

/**
 * Write to proc entries
 */
ssize_t proc_write(struct file *filp, const char *buf, size_t count, loff_t *offp) 
{
        char *data = NULL;
        char str[3];

        data = PDE_DATA(file_inode(filp));
        if(!(data)){
                printk(KERN_INFO "Null data");
                return 0;
        }
        
        if (!strncmp(data, proc_priv_data[2], strlen(proc_priv_data[2]))) {
                count = simple_write_to_buffer(str, 1, offp, buf, count) + 1;
                str[1] = '\0';
                if (!strcmp(str, "1"))
                        flush_io();
                else
                        count = -EINVAL;
        }
        else {
                /*Do nothing*/
                count = -EINVAL;
        }
        return count;
}

static const struct file_operations proc_fops = {
        .read = proc_read,
        .write = proc_write
};

/**
 * Create proc entries and add data to inode's priv field
 */
void create_proc_entries(void) 
{
        proc = proc_create_data("proc_1", S_IRWXU | S_IRWXG | S_IRWXO,
                                NULL, &proc_fops, proc_priv_data[0]);
        proc = proc_create_data("proc_2", S_IRWXU | S_IRWXG | S_IRWXO,
                                NULL, &proc_fops,proc_priv_data[1]);
        proc = proc_create_data("proc_3", S_IRWXU | S_IRWXG | S_IRWXO,
                                NULL, &proc_fops,proc_priv_data[2]);
        proc = proc_create_data("proc_4", S_IRWXU | S_IRWXG | S_IRWXO,
                                NULL, &proc_fops,proc_priv_data[3]);
}

/**
 * Removing proc entries 
 */
void remove_proc_entries(void)
{
        remove_proc_entry("proc_1", NULL);
        remove_proc_entry("proc_2", NULL);
        remove_proc_entry("proc_3", NULL);
        remove_proc_entry("proc_4", NULL);
}
