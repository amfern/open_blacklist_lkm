#include <linux/btree.h>
#include <linux/gfp.h>


#include "inode_blacklist.h"

struct btree_head btree_head;
void* dummy_value = (void *) 1;

int inode_blacklist_init() {
	return btree_init(&btree_head);
}

int inode_blacklist_init_populate(u64 *inodes, u64 length) {
	int ret;

	ret = inode_blacklist_init();

	if (ret) {
		return ret;
	}

	return inode_blacklist_populate(inodes, length);
}

void inode_blacklist_destroy(void) {
	btree_destroy(&btree_head);
}

int inode_blacklist_populate(u64 *inodes, u64 length) {
	int ret, i;

	for (i = 0; i < length; i++) {
		ret = btree_insert(&btree_head, &btree_geo64, (unsigned long int *)(inodes +i),
			dummy_value, GFP_KERNEL);

		if (ret) {
			return ret;
		}
	}

	return 0;
}

bool is_inode_blacklisted(u64 *inode) {
	return btree_lookup(&btree_head, &btree_geo64, (unsigned long int *)inode) != NULL;
}
