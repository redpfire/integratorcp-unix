
#include <inttype.h>

#define KSTATEADDR	0x500
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
} kstate_t;
