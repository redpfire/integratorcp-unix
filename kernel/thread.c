
#include <thread.h>
#include <kernel.h>
#include <stddef.h>
#include <mem.h>
#include <kvmm.h>

void flush_threads(kstate_t *);

void init_threading()
{
    kstate_t *ks;
    ks = (kstate_t *) KSTATEADDR;

    flush_threads(ks);

    ks->threadndx = 0x0;
    ks->iswitch = 1;
    init_heap(ks);
}

void flush_threads(kstate_t *ks)
{
    for(int x = 0 ; x < MAX_THREADS; ++x)
        ks->threads[x].valid = 0;
}

uint8_t getcurrentthread()
{
    kstate_t *ks;
    ks = (kstate_t *) KSTATEADDR;
    return ks->threadndx;
}

void create_thread(void *addr) // TODO: redo threads
{
    kstate_t *ks;
    ks = (kstate_t *) KSTATEADDR;
    int curr = -1;
    for(int i = 0 ; i < MAX_THREADS; ++i)
    {
        if(ks->threads[i].valid == 0)
        {
            curr = i;
            break;
        }
    }
    if(curr == -1)
    {
        // TODO: KERNEL PANIC
        return;
    }
    ks->threads[curr].valid = 1;
    ks->threads[curr].cpsr = 0x60000000 | ARM_MODE_USER;
    ks->threads[curr].sp = 0x90001000;
    ks->threads[curr].pc = 0x80000000;

    kvmm_init(&ks->threads[curr].vmm);
    kvmm_allocregion(ks->threads[curr].vmm, 0x80000000, 1, KVMM_USER | TLB_C_AP_FULLACCESS);
    kvmm_allocregion(ks->threads[curr].vmm, 0x90000000, 1, KVMM_USER | TLB_C_AP_FULLACCESS);

    // for testing purposes
    uint32_t page;
    kvmm_map(ks->threads[curr].vmm, 0xa0000000, 0x16000000, 1, TLB_C_AP_FULLACCESS);
    kvmm_getphy(ks->threads[curr].vmm, 0x80000000, &page);
    int x;
    for(x = 0; x < 24; ++x)
        ((uint32_t *)page)[x] = ((uint32_t *)addr)[x];
}
