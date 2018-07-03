
#include <thread.h>

void flush_threads(kstate_t *);

void init_threading()
{
    kstate_t *ks;
    ks = (kstate_t *) KSTATEADDR;

    flush_threads(ks);

    ks->threadndx = 0x0;
    ks->iswitch = 1;
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

void create_thread(void *addr, uint32_t sp, uint32_t ksp)
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
    ks->threads[curr].sp = sp;
    ks->threads[curr].ksp = ksp;
    ks->threads[curr].pc = (uint32_t)addr;
}
