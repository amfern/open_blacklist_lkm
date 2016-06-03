#include <linux/list.h>

struct blacklist_entry {
	struct list_head next;
	char *buf;
};

int parse_blacklist_file(char* blacklist_file, struct list_head *blacklist);
