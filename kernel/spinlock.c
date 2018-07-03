
#include <spinlock.h>
#include <thread.h>

void _access(mutex_t *mutex)
{
    while(mutex->lock);
    mutex->lock = true;
    mutex->thread = getcurrentthread();
}

void _leave(mutex_t *mutex)
{
    mutex->thread = 0x0;
    mutex->lock = false;
}
