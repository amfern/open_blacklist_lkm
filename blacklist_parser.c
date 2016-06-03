#include <linux/slab.h>     // kmalloc
#include <linux/string.h>
#include <linux/syscalls.h>

#include <linux/types.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/vmalloc.h>

#include "types.h"
#include "blacklist_parser.h"

int kernel_read(struct file *file, loff_t offset,
				char *addr, unsigned long count)
{
	mm_segment_t old_fs;
	loff_t pos = offset;
	int result;

	old_fs = get_fs();
	set_fs(get_ds());
	/* The cast to a user pointer is valid due to the set_fs() */
	result = vfs_read(file, (void __user *)addr, count, &pos);
	set_fs(old_fs);
	return result;
}


int kernel_read_file(struct file *file, void **buf, loff_t *size, loff_t max_size)
{
	loff_t i_size, pos;
	ssize_t bytes = 0;
	int ret = 0;

	if (!S_ISREG(file_inode(file)->i_mode) || max_size < 0)
		return -EINVAL;

	if (ret)
		return ret;

	ret = deny_write_access(file);
	if (ret)
		return ret;

	i_size = i_size_read(file_inode(file));
	if (max_size > 0 && i_size > max_size) {
		ret = -EFBIG;
		goto out;
	}
	if (i_size <= 0) {
		ret = -EINVAL;
		goto out;
	}

	*buf = vmalloc(i_size);
	if (!*buf) {
		ret = -ENOMEM;
		goto out;
	}

	pos = 0;
	while (pos < i_size) {
		bytes = kernel_read(file, pos, (char *)(*buf) + pos,
				    i_size - pos);
		if (bytes < 0) {
			ret = bytes;
			goto out;
		}

		if (bytes == 0)
			break;
		pos += bytes;
	}

	if (pos != i_size) {
		ret = -EIO;
		goto out_free;
	}

	if (!ret)
		*size = pos;

out_free:
	if (ret < 0) {
		vfree(*buf);
		*buf = NULL;
	}

out:
	allow_write_access(file);
	return ret;
}

int kernel_read_file_from_path(char *path, void **buf, loff_t *size, loff_t max_size)
{
	struct file *file;
	int ret;

	if (!path || !*path)
		return -EINVAL;

	file = filp_open(path, O_RDONLY, 0);
	if (IS_ERR(file))
		return PTR_ERR(file);

	ret = kernel_read_file(file, buf, size, max_size);
	fput(file);
	return ret;
}

int null_terminate_next_entry(char *datap) {
	char *replace_buf = datap;

	do {
		replace_buf++;
	} while (*replace_buf != '>');

	*replace_buf = '\0';

	return strlen(datap);
}

int parse_blacklist_file(char* blacklist_file, struct list_head *blacklist) {
	void *data;
	char *datap;
	loff_t size;
	int rc;

	rc = kernel_read_file_from_path(blacklist_file, &data, &size, 0);
	if (rc < 0) {
		pr_err("Unable to open file: %s (%d)", blacklist_file, rc);
		return rc;
	}

	datap = data;

	printk(KERN_DEBUG "datap = %s", datap);

	while (size > 0) {
		struct blacklist_entry *entry;

		rc = null_terminate_next_entry(datap);

		if (rc < 0)
			break;

		// skip the '<'
		datap++;
		size--;

		// add to blacklist
		entry = kmalloc(sizeof(struct blacklist_entry), GFP_KERNEL);
		entry->buf = kmalloc(rc, GFP_KERNEL);
		strcpy(entry->buf, datap);
		list_add(&entry->next, blacklist);

		printk(KERN_DEBUG "new datap entry = %s, size = %llu, rc = %d", datap, size, rc);

		// advance to next entry
		datap += rc;

		// skip the null terminator
		datap++;
		size--;

		// update the total size left
		size -= rc;
	}

	vfree(data);
	if (rc < 0)
		return rc;
	else if (size)
		return -EINVAL;
	else
		return strlen(blacklist_file);
}
