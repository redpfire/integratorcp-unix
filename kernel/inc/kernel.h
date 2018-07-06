
#include <inttype.h>

#define KSTACKSTART 0x2000
#define KSTACKEXC   0x3000
#define KSTATEADDR  0x3000

#define KRAMADDR 0x200000
#define KRAMSIZE (1024 * 1024 * 8)
/* 
	kernel virtual memory size 
 
	KMEMINDEX can be 1 through 7
	1 = 2GB
	2 = 1GB
	3 = 512MB
	4 = 256MB
	5 = 128MB
	6 = 64MB
	7 = 32MB
 
	This is the maximum amount of virtual memory per kernel space.
*/
#define KMEMINDEX 3
#define ENTRIES 512
#define KMEMSIZE (0x1000000 << (8 - KMEMINDEX))
// physical memory page size
#define KPHYPAGESIZE 4096
// block size of the chunk heap (kmalloc, kfree)
#define KCHKHEAPBSIZE 16
// minimum block size for chunk heap
#define KCHKMINBLOCKSZ (1024 * 1024)

void printk(const char *, ...);
void *kmalloc(uint32_t size);
void kfree(void *ptr);
