# Linux Block Device Driver
It is simple block device driver which caches every write IO. Once such THRESHOLD_IO_CNT of IO requests are buffered, those will be scheduled for actual flush to disk which will be handled workqueues. Slab pool is used for frequently allocated data structures.

Linux kernel concepts that are covered in this modules are:

  - slab allocator
  - workqueue
  - block device driver
  - porc fs
  - generic linked list

### Directory Heirarchy

* docs - contains design document
* include - header files
* src - source files
* test - tests

### Steps to compile and load module
**Warning**: code is tested on kernel 3.11.10

```sh
$ git clone [git-repo-url] ldd
$ cd ldd
$ make
$ sudo insmod module_ldd.ko threshold_io_count=2
```

Module also contains proc entries to give statistics regarding driver
* proc_1 - Displays total amount of memory used by driver 
* proc_2 - Batches of data flushed
* proc_3 - Forces driver to flush IO's from cache
* proc_4 - Data that is in memory & need to flush to disk

### Usage of proc entries
After inserting module, you can read/write to each proc entry depending upon what is supported. In any case it throws appropriate error. Following shows sample usage

```sh
$ cd ldd/tests
$ make
$ ./test 1 # Print driver statistics i.e data from proc entries
$ ./test 2 # Write to proc entries 
```
### Writing to device
Source code assumes /dev/sdc exists. You can write to device using echo, and dmesg wil show appropriate logs
```sh
$ echo "Hello World" > /dev/sdc0
$ dmesg
```

### License
GPLv3