
#include <mem.h>
#include <kernel.h>

void kfree(void *ptr)
{
    kstate_t *ks;
    ks = (kstate_t *)KSTATEADDR;
    k_heapBMFree(&ks->hchk, ptr);
}
