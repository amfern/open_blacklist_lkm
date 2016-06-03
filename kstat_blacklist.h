#ifndef __KSTAT_BLACKLIST_H
#define __KSTAT_BLACKLIST_H

#include <linux/stat.h>
#include <linux/btree.h>
#include <linux/list.h>

// TODO: maybe union?
struct kstat_tree_head {
	struct btree_head128 btree_head;
};

int kstat_tree_head_init(struct kstat_tree_head *tree);

void kstat_blacklist_destroy(struct kstat_tree_head *tree);

int kstat_blacklist_populate(struct kstat_tree_head *tree, struct kstat *blacklist, u64 length);

int kstat_blacklist_populate_path(struct kstat_tree_head *tree, char* blacklisted_paths[], u64 length);

bool is_kstat_blacklisted(struct kstat_tree_head *tree, struct kstat *stat);

#endif
