#include <linux/btree.h>
#include <linux/gfp.h>


#include "inode_blacklist.h"

struct btree_head128 btree_head;
void* dummy_value = (void *) 1;

int kstat_blacklist_init() {
	return btree_init128(&btree_head);
}

int kstat_blacklist_init_populate(struct kstat *blacklist, u64 length) {
	int ret;

	ret = kstat_blacklist_init();

	if (ret) {
		return ret;
	}

	return kstat_blacklist_populate(blacklist, length);
}

void kstat_blacklist_destroy(void) {
	btree_destroy128(&btree_head);
}

int kstat_blacklist_populate(struct kstat *blacklist, u64 length) {
	int ret, i;
	struct kstat stat;

	for (i = 0; i < length; i++) {
		stat = blacklist[i];
		ret = btree_insert128(&btree_head, stat.ino, stat.dev,
			dummy_value, GFP_KERNEL);

		if (ret) {
			return ret;
		}
	}

	return 0;
}

bool is_kstat_blacklisted(struct kstat *stat) {
	return btree_lookup128(&btree_head, stat->ino, stat->dev) != NULL;
}
