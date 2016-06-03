obj-m := blacklist.o
blacklist-objs := \
	kstat_blacklist.o \
	sys_open_hook.o \
	open_blacklist.o

# open_blacklist-y := sys_open_hook.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
