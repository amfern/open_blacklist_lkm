#include <linux/slab.h>     // kmalloc
#include <linux/string.h>
#include <linux/syscalls.h>

#include "types.h"
#include "blacklist_parser.h"

// TODO: split this into open func and parsing func
// TODO: make use of return value
int parse_blacklist_file(char* blacklist_file, struct list_head *blacklist) {
	struct file* filp = NULL;
	char *buf;
	int len;
	struct blacklist_entry *entry;

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
		char *replace_buf;

		len = vfs_read(filp, buf, PAGE_SIZE - 1, &pos);
		filp->f_pos = pos;

		if (len < 0) {
			printk(KERN_DEBUG "failed to read blacklist file");
			goto out_free;
		}

		buf[len] = '\0';

		replace_buf = buf;

		// break this string into collection of smaller string
		// representing the file paths
		while (*replace_buf != 0) {
			if (*replace_buf == '>') {
				*replace_buf = '\0';

				entry = kmalloc(sizeof(struct blacklist_entry), GFP_KERNEL);
				entry->buf = kmalloc(strlen(buf + 1) + 1, GFP_KERNEL);
				strcpy(entry->buf, buf + 1);
				list_add(&entry->next, blacklist);
			} else if (*replace_buf == '<') {
				buf = replace_buf;
			}
			replace_buf++;
		}

	} while (len > 0);

	list_for_each_entry(entry, blacklist, next) {
		printk(KERN_DEBUG "foreach entry = %s", entry->buf);
	}
out_free:
	kfree(buf);
out_fput:
	filp_close(filp, NULL);

	return 0;
}
