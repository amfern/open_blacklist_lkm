#ifndef __INODE_BLACKLIST_H
#define __INODE_BLACKLIST_H

int inode_blacklist_init(void);

int inode_blacklist_init_populate(u64 *inodes, u64 length);

void inode_blacklist_destroy(void);

int inode_blacklist_populate(u64 *inodes, u64 length);

bool is_inode_blacklisted(u64 *inode);

#endif
