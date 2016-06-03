#include <linux/module.h>
#include <linux/syscalls.h>

#include "sys_open_hook.h"
#include "kstat_blacklist.h"

/* Just so we do not taint the kernel */
MODULE_LICENSE("GPL");

char* blacklisted_paths[] = {
	"/home/amfern/Desktop"
};


// TODO: blacklist has to be repopulated upon any remount, otherwise dev_t
// and ino will be out of sync

long my_sys_open(const char __user *filename, int flags, int mode) {
    long ret;
	struct kstat stat;

	ret = call_sys_open(filename, flags, mode);

	vfs_fstat(ret, &stat);

	if (is_kstat_blacklisted(&stat)) {
		printk(KERN_DEBUG "file %s has been opened with mode %d", filename, mode);
	}

	return ret;
}

static inline int create_blacklist(void) {
	int ret;
	mm_segment_t fs;

	ret = kstat_blacklist_init();
	
	if (ret) {
		printk(KERN_DEBUG "failed to initialize blacklist");
		return ret;
	}

	// we have to run in kernel segment descriptor to get the
	// corrent stat
	fs = get_fs();
	// Set segment descriptor associated to kernel space
	set_fs(get_ds());

	ret = kstat_blacklist_populate_path(blacklisted_paths, 1);

	// restore user segment
	set_fs(fs);

	if (ret) {
		printk(KERN_DEBUG "failed to populate blacklist");
		return ret;
	}

	return 0;
}

static int __init syscall_init(void)
{
	int ret;

	printk(KERN_DEBUG "syscall_init");

	ret = create_blacklist();

	if (ret) {
		printk(KERN_DEBUG "failed to populate blacklist");
		return ret;
	}

	ret = override_sys_open(my_sys_open);

	if (ret) {
		printk(KERN_DEBUG "failed to override sys_open");
		return ret;
	}

	return 0;
}

static void __exit syscall_release(void)
{
	printk(KERN_DEBUG "syscall de-init");

	kstat_blacklist_destroy();
	restore_sys_open();
}

module_init(syscall_init);
module_exit(syscall_release);
