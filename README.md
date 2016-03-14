# Linux Block Device Driver
Its simple block device driver which caches every write IO. Once such THRESHOLD_IO_CNT of io requests are buffered, those will be scheduled for actual flush to disk which will be handled workqueues. Slab pool is used for frequently allocated data structures.

Linux kernel concepts that are covered in this modules are:

  - slab allocator
  - workqueue
  - block device driver
  - porc fs
  - generic linked list

In module created proc entries to give stats regarding driver
* proc_1 - Displays total ammount of memory used by driver 
* proc_2 - Batches of data flushed
* proc_3 - Forces driver to flush IO's from cache
* proc_4 - Data that is in memory & need to flush to disk

### Steps to compile and load module
**Warning**: code is tested on kernel 3.11.10

```sh
$ git clone [git-repo-url] ldd
$ cd ldd
$ make
$ sudo insmod module_ldd.ko threshold_io_count=2
```
### Usage of proc entries
After inserting module, you can read/write to each proc entry depending upon what is supported. Following shows sample usage
```sh
$ cd ldd/tests
$ make
$ ./test 1 # Print driver statistics i.e data from proc entries
$ ./test 2 # Write to proc entries 
```
### License
GPLv3


**Free Software, Hell Yeah!**
