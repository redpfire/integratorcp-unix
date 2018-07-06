
#include <kernel.h>
#include <stddef.h>
#include <inttype.h>
#include <mem.h>
#include <kvmm.h>
#include <serial.h>
 
/*
	kernel kernel page alloc
 
	- only alloc from hphy
*/
void *kkpalloc() {
	kstate_t	   *ks;
	int				x;
 
	ks = (kstate_t *)KSTATEADDR;
	return k_heapBMAlloc(&ks->hphy, KPHYPAGESIZE);
}
 
/*
	kernel user page alloc
 
	- first try to alloc from husr then hphy
*/
void *kupalloc() {
	kstate_t	   *ks;
	int				x;
	void			*p;
	char			buf[128];
 
	ks = (kstate_t *)KSTATEADDR;
	p = k_heapBMAlloc(&ks->husr, KPHYPAGESIZE); 
	if (!p) {
		p = k_heapBMAlloc(&ks->hphy, KPHYPAGESIZE);
		printk("p:0x%x\n", p);
	}
 
	return p;
}
 
/* Initialize the paging table structure. */
int kvmm_init(k_vmm_t *table) {
	uint32_t	x;
	kstate_t	*ks;
	char		buf[128];
 
	ks = (kstate_t *)KSTATEADDR;	
	table->table = (uint32_t *)k_heapBMAllocBound(&ks->hphy, 1024 * 16, 14);
 
	serial_putc('&');
 
	printk("table.table: 0x%x\n", table->table);
 
	for (x = 0; x < 1024 * 4; ++x)
		table->table[x] = 0;
}
 
/* Map contigious range starting at a virtual address and physical of count */
int kvmm_map(k_vmm_t table, uint32_t virtual, uint32_t physical, uint32_t count, uint32_t flags) {
	uint32_t		x, v, p, y;
	uint32_t		*t, *st;
	kstate_t		*ks;
	uint8_t	 		unmap;
 
	ks = (kstate_t *)KSTATEADDR;
 
	unmap = 0;
	if (flags & KVMM_UNMAP) {
		/* remove special flags */
		flags = flags & (~0 >> 1);
		unmap = 1;
	}
 
	/* this entire loop could be optimized, but it is simple and straight forward */
	v = virtual >> 12;
	p = physical >> 12;
	t = table.table;
	for (x = 0; x < count; ++x) {
		if ((t[v >> 8] & 3) == 0) {
			/* create table (alloc on 1KB boundary) */
			t[v >> 8] = (uint32_t)k_heapBMAllocBound(&ks->hphy, 1024 * 16, 10);
			if (!t[v >> 8]) {
				return 0;
			}
			/* get sub-table */
			st = (uint32_t*)t[v >> 8];
			t[v >> 8] |= TLB_COARSE;
			/* clear table (all entries throw a fault) */
			for (y = 0; y < 256; ++y) {
				st[y] = 0;
			}
		} else {
			/* get sub-table */
			st = (uint32_t *)(t[v >> 8] & ~0x3ff);
		}
 
		if ((st[v & 0xff] & 3) == 0) {
			/* map 4K page */
			if (!unmap) {
				/* map page */
				if (p + 1 == 0) {			
					st[v & 0xff] = (p << 12) | flags | TLB_C_SMALLPAGE;
				} else {
					st[v & 0xff] = (p << 12) | flags | TLB_C_SMALLPAGE;
				}
			} else {
				/* unmap page */
				st[v & 0xff] = 0;
			}
		}
 
		++v;
		++p;
	}
 
	return 1;
}
 
/* Find a region of size within specified boundries. */
int kvmm_getunusedregion(k_vmm_t table, uint32_t *virtual, uint32_t count, uint32_t lowerLimit, uint32_t upperLimit) {
	uint32_t		x, v, c, s;
	uint32_t		*t, *st;
	kstate_t		*ks;
 
	ks = (kstate_t*)KSTATEADDR;
 
	/* this entire loop could be optimized, but it is simple and straight forward */
	s = 0;	/* start */
	c = 0;  /* count */
	v = lowerLimit >> 12;
	t = table.table;
	for (x = 0; x < count; ++x) {
		if (t[v >> 8] & 3 == 0) {
			c++;
		} else {
			/* get sub-table */
			st = (uint32_t *)(t[v >> 8] & ~0x3ff);
 
			if (st[v & 0xff] & 3 == 0) {
				c++;
			} else {
				s = v << 12;
				c = 0;
			}
		}
 
		/* if we found a region of size */
		if (c >= count) {
			*virtual = s;
			return 1;
		}
 
		++v;
	}	
 
	*virtual = 0;
	return 0;
}
 
/*
	This does not support 64K or 16MB mappings, only 1MB and 4K.
*/
int kvmm_getphy(k_vmm_t table, uint32_t virtual, uint32_t *out) {
	uint32_t		*t;
	char			buf[128];
 
	/* not mapped */
	if ((table.table[virtual >> 20] & 3) == 0) {
		*out = 0;
		return 0;
	}
 
	printk("table.table[virtual >> 20]: 0x%x\n", table.table[virtual >> 20]);
 
	/* get 1MB section address */
	if ((table.table[virtual >> 20] & 3) == TLB_SECTION) {
		*out = table.table[virtual >> 20] & ~0xFFFFF;
		return 1;
	}
 
	/* get level 2 table */
	t = (uint32_t *)(table.table[virtual >> 20] & ~0x3ff);
 
	virtual = (virtual >> 12) & 0xFF;
 
	printk("t[%x]: 0x%x\n", virtual, t[virtual]);
 
	/* not mapped on level 2 */
	if ((t[virtual] & 3) == 0) {
		*out = 0;
		return 0;
	}
 
	/* get 4K mapping */
	*out = t[virtual] & ~0xfff;
	return 1;
}
 
/* Map fresh pages from physical page accounting system.*/
int kvmm_allocregion(k_vmm_t table, uint32_t virtual, uint32_t count, uint32_t flags) {
	uint32_t		x, v;
	uint32_t		*t, *st;
	kstate_t		*ks;
	uint8_t		kspace;
	uint32_t		y;
 
	/*
		If we are mapping in user space then lets try to use pages from the
		kusr map, and if for kernel then pull from kphy. 
	*/
	kspace = 1;
	if (flags & KVMM_USER) {
		kspace = 0;
	} else {
		if (!(flags & KVMM_KERNEL)) {
			/* force specification of either kernel space or user space mapping */
			return 0;
		}
	}
	/* remove flags */
	flags = flags & ~KVMM_KERNEL;
	flags = flags & ~KVMM_USER;
 
	ks = (kstate_t *)KSTATEADDR;
	/* this entire loop could be optimized, but it is simple and straight forward */
	v = virtual >> 12;
	t = table.table;
	for (x = 0; x < count; ++x) {
		if ((t[v >> 8] & 3) == 0) {
			/* create table (alloc on 1KB boundary) */
			t[v >> 8] = (uint32_t)k_heapBMAllocBound(&ks->hphy, 1024 * 16, 10);
			if (!t[v >> 8]) {
				/* memory failure */
				return 0;
			}
			st = (uint32_t *)t[v >> 8];
 
			t[v >> 8] |= TLB_COARSE;
 
			/* clear table (all entries throw a fault) */
			for (y = 0; y < 256; ++y) {
				st[y] = 0;
			}
		} else {
			/* get sub-table */
			st = (uint32_t *)(t[v >> 8] & ~0x3ff);
		}
 
		if ((st[v & 0xff] & 3) != 0) {
			/* return failure because we are mapping over already mapped memory */
			return 0;
		}
 
		/* map a page */
		if (kspace) {
			serial_putc('1');
			st[v & 0xff] = (uint32_t )kkpalloc() | flags | TLB_C_SMALLPAGE;
		} else {
			serial_putc('2');
			st[v & 0xff] = (uint32_t )kupalloc() | flags | TLB_C_SMALLPAGE;
		}
		/* increment to next virtual address */
		++v;
	}	
	return 1;	
}
/* Unmap pages and hand them back to the physical page accounting system. */
int kvmm_freeregion(k_vmm_t table, uint32_t virtual, uint32_t count) {
	uint32_t		x, v, p;
	uint32_t		*t, *st;
	kstate_t		*ks;
 
	ks = (kstate_t *)KSTATEADDR;
 
	/* this entire loop could be optimized, but it is simple and straight forward */
	v = virtual >> 12;
	t = table.table;
	for (x = 0; x < count; ++x, ++v) {
		if ((t[v >> 8] & 3) != 0) {
			/* get sub-table */
			st = (uint32_t *)(t[v >> 8] & ~0x3ff);
 
			/* unmap a page */
			if ((st[v & 0xff] & 3) != 0) {
				/* determine if it is a kernel or user page */
				p = (st[v & 0xff] >> 12 << 12);
				if (p > KMEMSIZE) {
					/* user */
					if (!k_heapBMFree(&ks->husr, (void*)p)) {
						return 0;
					}
				} else {
					/* kernel */
					if (!k_heapBMFree(&ks->hphy, (void*)p)) {
						return 0;
					}
				}
			}
		}
	}	
 
	return 1;
}