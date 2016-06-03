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

void kstat_tree_destroy(struct kstat_tree_head *tree);

int kstat_tree_insert(struct kstat_tree_head *tree, struct kstat *stat);

struct kstat* kstat_tree_remove(struct kstat_tree_head *tree, struct kstat *stat);

struct kstat* kstat_tree_lookup(struct kstat_tree_head *tree, struct kstat *stat);

int kstat_blacklist_populate(struct kstat_tree_head *tree, struct kstat *blacklist, u64 length);

int kstat_blacklist_populate_path(struct kstat_tree_head *tree, char* blacklisted_paths[], u64 length);

static inline bool kstat_tree_contain(struct kstat_tree_head *tree, struct kstat *stat) {
	return kstat_tree_lookup(tree, stat) != NULL;
}

#endif
