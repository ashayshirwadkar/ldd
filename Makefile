obj-m := module_ldd.o
module_ldd-objs :=  ldd.o proc_entries.o queue.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

#obj-m := ldd.o
#KDIR := /lib/modules/$(shell uname -r)/build
#PWD := $(shell pwd)
#default:
#	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
