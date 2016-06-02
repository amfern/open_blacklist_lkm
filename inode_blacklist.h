#ifndef __KSTAT_BLACKLIST_H
#define __KSTAT_BLACKLIST_H

#include <linux/stat.h>

int kstat_blacklist_init(void);

int kstat_blacklist_init_populate(struct kstat *blacklist, u64 length);

void kstat_blacklist_destroy(void);

int kstat_blacklist_populate(struct kstat *blacklist, u64 length);

bool is_kstat_blacklisted(struct kstat *stat);

#endif
