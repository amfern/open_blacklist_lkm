#ifndef __BLACKLIST_LKM_TYPES_H
#define __BLACKLIST_LKM_TYPES_H

#include <linux/list.h>

struct blacklist_entry {
	struct list_head next;
	char *buf;
};

#endif
