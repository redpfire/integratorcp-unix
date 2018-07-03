
#include <mem.h>
#include <inttype.h>

void memset(void *dest, int ch, int count)
{
    uint32_t c = (uint32_t) ch;
    uint32_t *dst = (uint32_t *)dest;
    for(int i = 0 ; i < count ; ++i)
        *(dst+i) = c;
}
