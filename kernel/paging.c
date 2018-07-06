
#include <paging.h>
#include <kernel.h>
#include <kvmm.h>

extern int __start;
extern int __end;

void init_paging(kstate_t *ks)
{
    int x;
    // ktlb = k_heapBMAllocBound(&ks->hphy, 1024 * 16, 14);
    //utlb = k_heapBMAllocBound(&ks->hphy, 1024 * 16, 14);

    arm_tlbsetmode(KMEMINDEX);

    // for(x = 0 ; x < ENTRIES; ++x)
    //     ktlb[x] = (x << 20) | TLB_AP_PRIVACCESS | TLB_SECTION;
    
    // utlb[0x800] = (1 << 20) | TLB_AP_PRIVACCESS | TLB_SECTION;
    // utlb[0x801] = (1 << 20) | TLB_AP_PRIVACCESS | TLB_SECTION;
    // ktlb[1] = (2 << 20) | TLB_AP_PRIVACCESS | TLB_SECTION;
    // ktlb[2] = (2 << 20) | TLB_AP_PRIVACCESS | TLB_SECTION;

    debugger();

    kvmm_init(&ks->vmmk);
    for(x = 0 ;x < (KRAMADDR >> 20); ++x)
        ks->vmmk.table[x] = (x << 20) | TLB_AP_PRIVACCESS | TLB_SECTION;
    kvmm_map(ks->vmmk, (uint32_t)&__start, (uint32_t)&__start, (uint32_t)&__end - (uint32_t)&__start, TLB_C_AP_PRIVACCESS);
    kvmm_map(ks->vmmk, 0, 0, 1, TLB_C_AP_PRIVACCESS);
    kvmm_map(ks->vmmk, 0x16000000, 0x16000000, 1, TLB_C_AP_PRIVACCESS);
    kvmm_map(ks->vmmk, 0x13000000, 0x13000000, 1, TLB_C_AP_PRIVACCESS);
    kvmm_map(ks->vmmk, 0x14000000, 0x14000000, 1, TLB_C_AP_PRIVACCESS);

    arm_tlbset0((uint32_t)ks->vmmk.table);
    arm_tlbset1((uint32_t)ks->vmmk.table);

    printk("OK\n");

    //arm_tlbset1((uint32_t)utlb);
    // arm_tlbset0((uint32_t)ktlb);
    arm_tlbsetdom(0x55555555);
    arm_tlbsetctrl(arm_tlbgetctrl() | 0x800001);
    
    printk("[PTL] Paging enabled!\n");

}
