obj-m := blacklist.o
blacklist-objs := \
	kstat_tree.o \
	sys_open_hook.o \
	blacklist_parser.o \
	open_blacklist.o

# open_blacklist-y := sys_open_hook.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
