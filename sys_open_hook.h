#ifndef __SYS_OPEN_HOOK_H
#define __SYS_OPEN_HOOK_H

unsigned long **find_sys_call_table(void);

long call_sys_open(const char __user *filename, int flags, int mode);

int override_sys_open(void* sys_open);

int restore_sys_open(void);

#endif

