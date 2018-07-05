
#include <mem.h>
#include <kernel.h>

void *kmalloc(uint32_t size)
{
    void *ptr;
    kstate_t *ks;
    uint32_t _size;

    ks = (kstate_t *) KSTATEADDR;

    ptr = k_heapBMAlloc(&ks->hchk, size);
    if(!ptr)
    {
        if(size < KCHKMINBLOCKSZ / 2)
        {
            _size = KCHKMINBLOCKSZ;
        }
        else
        {
            _size = size * 2;
            _size = (_size / KPHYPAGESIZE) * KPHYPAGESIZE < _size ? _size / KPHYPAGESIZE + 1 : _size / KPHYPAGESIZE;
            _size = _size * KPHYPAGESIZE;
        }
        ptr = k_heapBMAlloc(&ks->hphy, _size);
        if(!ptr)
            return 0;
        k_heapBMAddBlock(&ks->hchk, (uint32_t)ptr, _size, KCHKHEAPBSIZE);
        ptr = k_heapBMAlloc(&ks->hchk, size);
    }
    return ptr;
}
