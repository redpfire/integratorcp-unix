
#include <mem.h>
#include <inttype.h>

void *memset(void *dest, int ch, int count)
{
    uint8_t *dst = (uint8_t *)dest;
    while(count--)
        *dst++ = ch;
    return dest;
}
