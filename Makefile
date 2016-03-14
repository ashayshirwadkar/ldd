obj-m := module_ldd.o
module_ldd-objs := src/ldd.o src/proc_entries.o src/queue.o
ccflags-y := -I$(PWD)/include
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
