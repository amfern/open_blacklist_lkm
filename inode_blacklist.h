#ifndef __KSTAT_BLACKLIST_H
#define __KSTAT_BLACKLIST_H

#include <linux/stat.h>

int kstat_blacklist_init(void);

void kstat_blacklist_destroy(void);

int kstat_blacklist_populate(struct kstat *blacklist, u64 length);

int kstat_blacklist_populate_path(char* blacklisted_paths[], u64 length);

bool is_kstat_blacklisted(struct kstat *stat);

#endif
