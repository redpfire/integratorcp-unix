
#include <paging.h>
#include <kernel.h>
#include <tlb.h>

void init_paging(kstate_t *ks)
{
    uint32_t *ktlb;
    uint32_t *utlb;
    int x;
    uint32_t *a;
    uint32_t *b;
    ktlb = k_heapBMAllocBound(&ks->hphy, 1024 * 16, 14);
    utlb = k_heapBMAllocBound(&ks->hphy, 1024 * 16, 14);

    arm_tlbsetmode(1);

    for(x = 0 ; x < 2048; ++x)
        ktlb[x] = (x << 2) | TLB_AP_PRIVACCESS | TLB_SECTION;
    
    utlb[0x800] = (1 << 20) | TLB_AP_PRIVACCESS | TLB_SECTION;
    utlb[0x801] = (1 << 20) | TLB_AP_PRIVACCESS | TLB_SECTION;
    ktlb[1] = (2 << 20) | TLB_AP_PRIVACCESS | TLB_SECTION;
    ktlb[2] = (2 << 20) | TLB_AP_PRIVACCESS | TLB_SECTION;

    printk("OK\n");

    arm_tlbset1((uint32_t)utlb);
    arm_tlbset0((uint32_t)ktlb);
    arm_tlbsetdom(0x55555555);
    arm_tlbsetctrl(arm_tlbgetctrl() | 0x800001);

    printk("OK2\n");

    a = (uint32_t *)0x80000000;
    b = (uint32_t *)0x80100000;
    a[0] = 0x12345678;
    printk("utlb: 0x%x\n", b[0]);

    a = (uint32_t *)0x100000;
    b = (uint32_t *)0x200000;
    a[0] = 0x87654321;
    printk("ktlb: 0x%x\n", b[0]);
    
    printk("paging on!\n");
}
