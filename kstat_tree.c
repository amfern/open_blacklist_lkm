#include <linux/slab.h>     // kmalloc
#include <linux/syscalls.h>

#include "kstat_tree.h"


int kstat_tree_head_init(struct kstat_tree_head *tree) {
	return btree_init128(&tree->btree_head);
	
}

void kstat_tree_destroy(struct kstat_tree_head *tree) {
	btree_destroy128(&tree->btree_head);
}

int kstat_tree_insert(struct kstat_tree_head *tree, struct kstat *stat)
{
	return btree_insert128(&tree->btree_head, stat->ino, stat->dev,
						  stat, GFP_KERNEL);
}

struct kstat* kstat_tree_remove(struct kstat_tree_head *tree, struct kstat *stat)
{
	return btree_remove128(&tree->btree_head, stat->ino, stat->dev);
}

struct kstat* kstat_tree_lookup(struct kstat_tree_head *tree, struct kstat *stat) {
	return btree_lookup128(&tree->btree_head, stat->ino, stat->dev);
}

int kstat_blacklist_populate(struct kstat_tree_head *tree, struct kstat *blacklist, u64 length) {
	int ret = 0;
	u64 i;
	struct kstat stat;

	for (i = 0; i < length; i++) {
		stat = blacklist[i];
		printk(KERN_DEBUG "stat.ino = %llu, stat.dev = %d", stat.ino, stat.dev);

		ret = kstat_tree_insert(tree, &stat);

		if (ret) {
			return ret;
		}
	}

	return 0;
}

int kstat_blacklist_populate_path(struct kstat_tree_head *tree, char* blacklisted_paths[], u64 length) {
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

	kstat_blacklist_populate(tree, blacklisted_stats, length);

	kfree(blacklisted_stats);

	return 0;
}

