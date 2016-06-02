# ccflags-y :=  -I$(src)

obj-m := blacklist.o
blacklist-objs := \
	inode_blacklist.o \
	sys_open_hook.o \
	open_blacklist.o

# open_blacklist-y := sys_open_hook.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean




# check if table is exported already and warn the user, ask the user to run with force
# create file in /proc/ for storing the blacklisted paths and watch for changes on that file
# don't forget files under dir
# should i use inode to validate symlinks
# explore LD_PRELOAD path, will not always work
