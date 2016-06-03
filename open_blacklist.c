#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/syscalls.h>

#include "types.h"
#include "sys_open_hook.h"
#include "kstat_tree.h"
#include "blacklist_parser.h"

/* Just so we do not taint the kernel */
MODULE_LICENSE("GPL");

static char *blacklist_file = "blah";
static struct kstat_tree_head kstat_tree_blacklist;

module_param(blacklist_file, charp, 0000);
MODULE_PARM_DESC(blacklist_file, "File listing the blacklisted paths");

char* blacklisted_paths[] = {
	"/home/amfern/Desktop"
};


// TODO: blacklist has to be repopulated upon any remount, otherwise dev_t
// and ino will be out of sync

long my_sys_open(const char __user *filename, int flags, int mode) {
    long ret;
	struct kstat stat;

	ret = call_sys_open(filename, flags, mode);

	// the return from call_sys_open is file descriptor
	vfs_fstat(ret, &stat);

	if (is_kstat_blacklisted(&kstat_tree_blacklist, &stat)) {
		printk(KERN_DEBUG "file %s has been opened with mode %d", filename, mode);
	}

	return ret;
}

static inline int create_blacklist(struct kstat_tree_head *kstat_tree) {
	int ret;
	mm_segment_t fs;

	LIST_HEAD(blacklist); // TODO: release the list, look how other
						  // doing it
	// we have to run in kernel segment descriptor to get the
	// corrent stat
	fs = get_fs();
	// Set segment descriptor associated to kernel space
	set_fs(get_ds());

	// parse file and return blacklist of type list
	parse_blacklist_file(blacklist_file, &blacklist);

	// create kstat tree from the blacklist
	ret = kstat_tree_head_init(kstat_tree);

	if (ret) {
		printk(KERN_DEBUG "failed to initialize blacklist");
		return ret;
	}

	ret = kstat_blacklist_populate_path(kstat_tree, blacklisted_paths, 1);
	// ret = kstat_blacklist_populate_path_list(kstat_tree, blacklist);

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

	ret = create_blacklist(&kstat_tree_blacklist);

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

	kstat_blacklist_destroy(&kstat_tree_blacklist);
	restore_sys_open();
}

module_init(syscall_init);
module_exit(syscall_release);
