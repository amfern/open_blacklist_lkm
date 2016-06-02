#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/slab.h>     // kmalloc

#include "sys_open_hook.h"
#include "inode_blacklist.h"

/* Just so we do not taint the kernel */
MODULE_LICENSE("GPL");

/* static inline int same_file(int fd1, int fd2) { */
/*     struct stat stat1, stat2; */
/*     if(fstat(fd1, &stat1) < 0) return -1; */
/*     if(fstat(fd2, &stat2) < 0) return -1; */
/*     return (stat1.st_dev == stat2.st_dev) && (stat1.st_ino == stat2.st_ino); */
/* } */

u64 blaclisted_inodes[] = {
	655368,
	655372,
	655369,
	656747,
	655373,
	655374,
	655371,
	655370,
	655375,
	656232
};

long my_sys_open(const char __user *filename, int flags, int mode) {
    long ret;
	struct kstat *stat;
	stat = kmalloc(sizeof (struct kstat), GFP_KERNEL);

	vfs_stat(filename, stat);

	if (!stat) {
		/* the allocation failed - handle appropriately */
		printk(KERN_DEBUG "failed allocate memory QQ");
	}
	
	ret = call_sys_open(filename, flags, mode);
	
	if (is_inode_blacklisted( &(stat->ino) )) {
		printk(KERN_DEBUG "file %s has been opened with mode %d, ino %llu", filename, mode, stat->ino);
	}

	kfree(stat);

	return ret;
}

static int __init syscall_init(void)
{
	int ret;

	printk(KERN_DEBUG "syscall_init\n");

	ret = inode_blacklist_init_populate(blaclisted_inodes, sizeof(blaclisted_inodes) / sizeof(u64));

	if (ret) {
		printk(KERN_DEBUG "failed to initialize blacklist_collection");
		return ret;
	}

	return override_sys_open(my_sys_open);
}

static void __exit syscall_release(void)
{
	printk(KERN_DEBUG "syscall de-init\n");

	inode_blacklist_destroy();
	restore_sys_open();
}

module_init(syscall_init);
module_exit(syscall_release);
