#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/syscalls.h>

#include <linux/list.h>
#include <linux/slab.h>     // kmalloc
#include <linux/string.h>


#include "sys_open_hook.h"
#include "kstat_blacklist.h"

/* Just so we do not taint the kernel */
MODULE_LICENSE("GPL");

static char *blacklist_file = "blah";

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

struct blacklist_entry {
	struct list_head next;
	char *buf;
};

int parse_blacklist_file(char* blacklist_file, char** blacklist, u64 *blacklist_length) {
	struct file* filp = NULL;
	char *buf;
	int len;
	struct blacklist_entry *entry;

	LIST_HEAD(blacklist_list);

	filp = filp_open(blacklist_file, O_RDONLY, 0);
    if(IS_ERR(filp)) {
		printk(KERN_DEBUG "failed to open blacklist file");
        return PTR_ERR(filp);
    }

	buf = kmalloc(PAGE_SIZE, GFP_KERNEL);

	if (buf == NULL) {
		printk(KERN_DEBUG "Failed to allocate buffer");
		goto out_fput;
	}

	do {
		loff_t pos = filp->f_pos;
		mm_segment_t old_fs = get_fs();
		char *replace_buf;

		set_fs(KERNEL_DS);
		len = vfs_read(filp, buf, PAGE_SIZE - 1, &pos);
		set_fs(old_fs);
		filp->f_pos = pos;

		if (len < 0) {
			printk(KERN_DEBUG "failed to read blacklist file");
			goto out_free;
		}

		// buf[len] = '\0';

		replace_buf = buf;

		// break this string into collection of smaller string
		// representing the file paths
		while (*replace_buf != 0) {
			if (*replace_buf == '>') {
				*replace_buf = '\0';

				entry = kmalloc(sizeof(struct blacklist_entry), GFP_KERNEL);
				entry->buf = kmalloc(strlen(buf + 1) + 1, GFP_KERNEL);
				strcpy(entry->buf, buf + 1);
				list_add(&entry->next, &blacklist_list);

				// printk(KERN_DEBUG "buffer = %s ###############", buf + 1);
			} else if (*replace_buf == '<') {
				buf = replace_buf;
			}
			replace_buf++;
		}

	} while (len > 0);

	list_for_each_entry(entry, &blacklist_list, next) {
		printk(KERN_DEBUG "foreach entry = %s", entry->buf);
	}
out_free:
	kfree(buf);
out_fput:
	filp_close(filp, NULL);
	// fput(filp);


/* 	/\** */
/* 	 * list_for_each_entry	-	iterate over list of given type */
/* 	 * @pos:	the type * to use as a loop cursor. */
/* 	 * @head:	the head for your list. */
/* 	 * @member:	the name of the list_head within the struct. */
/* 	 *\/ */
/* #define list_for_each_entry(pos, head, member)					\ */
/* 	for (pos = list_first_entry(head, typeof(*pos), member);	\ */
/* 	     &pos->member != (head);								\ */
/* 	     pos = list_next_entry(pos, member)) */

	return 0;
}

static int __init syscall_init(void)
{
	int ret;
	char** blacklist = NULL;
	u64 *blacklist_length = NULL;

	printk(KERN_DEBUG "syscall_init");
	
	parse_blacklist_file(blacklist_file, blacklist, blacklist_length);

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
