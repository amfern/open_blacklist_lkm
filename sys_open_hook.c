#include <linux/syscalls.h>
#include <linux/delay.h>    // loops_per_jiffy
#include <linux/slab.h>     // kmalloc

#include "sys_open_hook.h"

#define CR0_WP 0x00010000   // Write Protect Bit (CR0:16)

long (*orig_sys_open)(const char __user *filename, int flags, int mode);

void **syscall_table;

long call_sys_open(const char __user *filename, int flags, int mode) {
	return orig_sys_open(filename, flags, mode);
}

/* Make the page writable */
int make_rw(unsigned long address)
{
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);
	if(pte->pte &~ _PAGE_RW)
		pte->pte |= _PAGE_RW;
	return 0;
}

/* Make the page write protected */
int make_ro(unsigned long address)
{
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);
	pte->pte = pte->pte &~ _PAGE_RW;
	return 0;
}

/**
 * Addresses in my System.map file
 * ffffffff81175dc0 T sys_close
 * ffffffff81801300 R sys_call_table
 * ffffffff81c0f3a0 D loops_per_jiffy
 */
unsigned long **find_sys_call_table()
{
    unsigned long ptr;
    unsigned long *p;

    for (ptr = (unsigned long)sys_close;
         ptr < (unsigned long)&loops_per_jiffy;
         ptr += sizeof(void *)) {
             
        p = (unsigned long *)ptr;

        if (p[__NR_close] == (unsigned long)sys_close) {
            printk(KERN_DEBUG "Found the sys_call_table!!!\n");
            return (unsigned long **)p;
        }
    }
    
    return NULL;
}

int override_sys_open(void* sys_open)
{
    int ret;
    unsigned long addr;
    unsigned long cr0;

	if (syscall_table == NULL) {
		syscall_table = (void **)find_sys_call_table();
	}

    if (!syscall_table) {
        printk(KERN_DEBUG "Cannot find the system call address\n"); 
        return -1;
    }

	// Remove write protection from kernel pages
    cr0 = read_cr0();
    write_cr0(cr0 & ~CR0_WP);

	// Make syscall_table address page rw
    addr = (unsigned long)syscall_table;
	ret = make_rw(addr);

	if(ret) {
        printk(KERN_DEBUG "Cannot set the memory to rw (%d) at addr %16lX\n", ret, PAGE_ALIGN(addr) - PAGE_SIZE);
    } else {
        printk(KERN_DEBUG "page set to rw");
    }

	orig_sys_open = syscall_table[__NR_open];
    syscall_table[__NR_open] = sys_open;

	// restore ro to syscall table page
	ret = make_ro(addr);

	if(ret) {
        printk(KERN_DEBUG "Cannot set the memory to ro (%d) at addr %16lX\n", ret, PAGE_ALIGN(addr) - PAGE_SIZE);
    } else {
        printk(KERN_DEBUG "page set to rw");
    }

	// restore write protection for kernel pages
    write_cr0(cr0);
  
    return 0;	
}

int restore_sys_open() {
	return override_sys_open(orig_sys_open);
}

