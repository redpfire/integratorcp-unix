
#include <kernel.h>
#include <paging.h>

void k_heapBMInit(k_heapbm_t *heap)
{
    heap->fblock = 0;
}

int k_heapBMAddBlock(k_heapbm_t *heap, uint32_t addr, uint32_t size, uint32_t bsize)
{
    k_heapblockbm_t *b;
    uint32_t bmsz;
    uint8_t *bm;

    b = (k_heapblockbm_t *)addr;
    bmsz = k_heapBMGetBMSize(size, bsize);
    bm = (uint8_t *)(addr + sizeof(k_heapblockbm_t));
    return k_heapBMAddBlockEx(heap, addr + sizeof(k_heapblockbm_t), size - sizeof(k_heapblockbm_t), bsize, b, bm, 1);
}

uint32_t k_heapBMGetBMSize(uint32_t size, uint32_t bsize)
{
    return size / bsize;
}

int k_heapBMAddBlockEx(k_heapbm_t *heap, uint32_t addr, uint32_t size, uint32_t bsize, k_heapblockbm_t *b, uint8_t *bm, uint8_t isBMInside)
{
    uint32_t bcnt;
    uint32_t x;
    b->size = size;
    b->bsize = bsize;
    b->data = addr;
    b->bm = bm;
    b->next = heap->fblock;
    heap->fblock = b;
    bcnt = size / bsize;
    //clear bitmap
    for(x = 0 ; x < bcnt; ++x)
    {
        bm[x] = 0;
    }
    bcnt = (bcnt / bsize) * bsize < bcnt ? bcnt / bsize + 1 : bcnt / bsize;

    if(isBMInside)
    {
        // reserve room for bitmap
        for(x = 0 ; x < bcnt; ++x)
        {
            bm[x] = 5;
        }
    }

    b->lfb = bcnt - 1;
    b->used = bcnt;
    return 1;
}

static uint8_t k_heapBMGetNID(uint8_t a, uint8_t b)
{
    uint8_t c;
    for (c = a + 1 ; c == b || c == 0; ++c);
    return c;
}

void *k_heapBMAlloc(k_heapbm_t *heap, uint32_t size)
{
    return k_heapBMAllocBound(heap, size, 0);
}

void *k_heapBMAllocBound(k_heapbm_t *heap, uint32_t size, uint32_t bound)
{
    k_heapblockbm_t *b;
    uint8_t *bm;
    uint32_t bcnt;
    uint32_t x, y, z;
    uint32_t bneed;
    uint8_t nid;
    uint32_t max;

    bound = ~(~0 << bound);

    for(b = heap->fblock; b; b = b ->next)
    {
        if(b->size - (b->used * b->bsize) >= size)
        {
            bcnt = b->size / b->bsize;
            bneed = (size / b->bsize) * b->bsize < size ? size / b->bsize + 1 : size / b->bsize;
            bm = (uint8_t *)b->bm;

            for(x = (b->lfb + 1 >= bcnt ? 0 : b->lfb + 1); x != b->lfb; ++x)
            {
                if(x >= bcnt)
                    x = 0;
                
                if ((((x * b->bsize) + b->data) & bound) != 0)
					continue;
                
                if(bm[x] == 0)
                {
                    //count free blocks
                    max = bcnt - x;
                    for(y = 0 ; bm[x + y] == 0 && y < bneed && y < max; ++y);

                    if(y == bneed)
                    {
                        nid = k_heapBMGetNID(bm[x - 1], bm [x + y]);

                        for(z = 0; z < y; ++z)
                            bm[x + z] = nid;

                        b->lfb = (x + bneed) - 2;
                        //count used blocks
                        b->used += y;
                        return (void*)((x * b->bsize) + b->data);
                    }

                    x += (y - 1);
                    continue;
                }
            }
        }
    }

    return 0;
}

void k_heapBMSet(k_heapbm_t *heap, uint32_t ptr, uint32_t size, uint8_t rval)
{
    k_heapblockbm_t *b;
    uint32_t ptroff, endoff;
    uint32_t bi, x ,ei;
    uint8_t *bm;
    uint8_t id;
    uint32_t max;

    for(b = heap->fblock; b; b = b->next)
    {
        if (
			/* head end resides inside block */
			(ptr >= b->data && ptr < b->data + b->size) ||
			/* tail end resides inside block */
			((ptr + size) >= b->data && (ptr + size) < b->data + b->size) ||
			/* spans across but does not start or end in block */
			(ptr < b->data && (ptr + size) > b->data + b->size)
		) 
        {
            if(ptr >= b->data)
            {
                ptroff = ptr - b->data;
                bi = ptroff / b->bsize;
            }
            else
                bi = 0;
            
            bm = b->bm;
            ptr = ptr + size;
            endoff = ptr - b->data;
            ei = (endoff / b->bsize) * b->bsize < endoff ? (endoff / b->bsize) + 1 : endoff / b->bsize;
            ++ei;

            max = b->size / b->bsize;
            max = ei > max ? max : ei;

            for(x = bi; x < max; ++x)
                bm[x] = rval;
            
            if(rval ==0)
                b->used -= ei - bi;
            else  
                b->used += ei - bi;
        }
    }
    return;
}

bool k_heapBMFree(k_heapbm_t *heap, void *ptr)
{
    k_heapblockbm_t *b;
    uint32_t ptroff;
    uint32_t bi, x;
    uint8_t *bm;
    uint8_t id;
    uint32_t max;

    for(b = heap->fblock; b; b = b->next)
    {
        if((uint32_t) ptr > b->data && (uint32_t) ptr < b->data + b->size)
        {
            ptroff = (uint32_t)ptr - b->data;
            bi = ptroff / b->bsize;
            bm = b->bm;
            id = bm[bi];
            max = b->size / b->bsize;
            for(x = bi; bm[x] == id && x < max; ++x)
                bm[x] = 0;
            b->used -= x - bi;
            return true;
        }
    }
    return false;
}

extern int __end;
extern int __start;

void init_heap(kstate_t *ks)
{
    uint8_t *bm;
    k_heapBMInit(&ks->hphy);
    k_heapBMInit(&ks->hchk);

    k_heapBMAddBlock(&ks->hchk, KRAMADDR, sizeof(kstate_t), 5);

    // state structure
    k_heapBMSet(&ks->hchk, KSTATEADDR, sizeof(kstate_t), 5);
    // stacks
    k_heapBMSet(&ks->hchk, KSTACKSTART - 0x1000, 0x1000, 6);
    k_heapBMSet(&ks->hchk, KSTACKEXC - 0x1000, 0x1000, 7);

    bm = (uint8_t *)k_heapBMAlloc(&ks->hchk, k_heapBMGetBMSize(KRAMSIZE - KRAMADDR, KPHYPAGESIZE));
    k_heapBMAddBlockEx(&ks->hphy, KRAMADDR, KRAMSIZE - KRAMADDR, KPHYPAGESIZE, (k_heapblockbm_t *)k_heapBMAlloc(&ks->hchk, sizeof(k_heapblockbm_t)), bm ,0);

    // map out kernel image

    k_heapBMSet(&ks->hphy, (uint32_t)&__start, (uint32_t)&__end - (uint32_t)&__start, 8);
    k_heapBMSet(&ks->hchk, (uint32_t)&__start, (uint32_t)&__end - (uint32_t)&__start, 8);

    // map out interrupt table

    k_heapBMSet(&ks->hphy, 0x0, 0x1C, 9);
    k_heapBMSet(&ks->hchk, 0x0, 0x1C, 9);
    init_paging(ks);
}
