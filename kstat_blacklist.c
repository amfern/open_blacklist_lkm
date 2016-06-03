#include <linux/btree.h>
#include <linux/gfp.h>
#include <linux/slab.h>     // kmalloc
#include <linux/syscalls.h>

#include "kstat_blacklist.h"

struct btree_head128 btree_head;
void* dummy_value = (void *) 1;

int kstat_blacklist_init() {
	return btree_init128(&btree_head);
}

void kstat_blacklist_destroy(void) {
	btree_destroy128(&btree_head);
}

int kstat_blacklist_populate(struct kstat *blacklist, u64 length) {
	int ret = 0;
	u64 i;
	struct kstat stat;

	for (i = 0; i < length; i++) {
		stat = blacklist[i];
		printk(KERN_DEBUG "stat.ino = %llu, stat.dev = %d", stat.ino, stat.dev);

		ret = btree_insert128(&btree_head, stat.ino, stat.dev,
			dummy_value, GFP_KERNEL);

		if (ret) {
			return ret;
		}
	}

	return 0;
}

int kstat_blacklist_populate_path(char* blacklisted_paths[], u64 length) {
	// int ret;
	u64 i;
	struct kstat *blacklisted_stats;
	struct kstat *stat;

	blacklisted_stats = kmalloc_array(length ,sizeof(struct kstat), GFP_KERNEL);
	for (i = 0; i < length; i++) {
		stat = &blacklisted_stats[i];
		
		vfs_stat(blacklisted_paths[i], stat); // TODO: what suppose
											  // the return value represent?

		printk(KERN_DEBUG "kstat_blacklist_populate_path: blacklisted_paths[i] = %s, stat.ino = %llu, stat.dev = %d", blacklisted_paths[i], stat->ino, stat->dev);
	}

	kstat_blacklist_populate(blacklisted_stats, length);

	kfree(blacklisted_stats);

	return 0;
}

bool is_kstat_blacklisted(struct kstat *stat) {
	return btree_lookup128(&btree_head, stat->ino, stat->dev) != NULL;
}
