
#include <inttype.h>

typedef struct
{
    uint32_t *table;
} k_vmm_t;

typedef struct k_heapblockbm
{
    struct k_heapblockbm *next;
    uint32_t size;
    uint32_t used;
    uint32_t bsize;
    uint32_t lfb;
    uint32_t data;
    uint8_t *bm;
} k_heapblockbm_t;

typedef struct
{
    k_heapblockbm_t *fblock;
} k_heapbm_t;

#define MAX_THREADS 0x10

typedef struct
{
    uint8_t valid;
    uint32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, sp, ksp, lr, cpsr, pc;
} thread_t;

typedef struct
{
    thread_t threads[MAX_THREADS];
    uint8_t iswitch;
    uint8_t threadndx;
    k_heapbm_t hphy; // physical page heap
    k_heapbm_t hchk; // data chunk heap
    k_heapbm_t husr;
} kstate_t;

void k_heapBMInit(k_heapbm_t *heap);
int k_heapBMAddBlock(k_heapbm_t *heap, uint32_t addr, uint32_t size, uint32_t bsize);
int k_heapBMAddBlockEx(k_heapbm_t *heap, uint32_t addr, uint32_t size, uint32_t bsize, k_heapblockbm_t *b, uint8_t *bm, uint8_t isBMInside);
void *k_heapBMAlloc(k_heapbm_t *heap, uint32_t size);
void *k_heapBMAllocBound(k_heapbm_t *heap, uint32_t size, uint32_t bound);
bool k_heapBMFree(k_heapbm_t *heap, void *ptr);
uint32_t k_heapBMGetBMSize(uint32_t size, uint32_t bsize);
void *k_heapBMAllocBound(k_heapbm_t *heap, uint32_t size, uint32_t mask);
void k_heapBMSet(k_heapbm_t *heap, uint32_t ptr, uint32_t size, uint8_t rval);

void init_heap(kstate_t *ks);

void *memset(void *dest, int ch, int count);
