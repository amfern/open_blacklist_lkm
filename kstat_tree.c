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
