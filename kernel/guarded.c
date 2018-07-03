
#include <guarded.h>
#include <serial.h>

void init_guarded()
{
    resources_t *r = (resources_t *)GUARDED_POINTER;
    for(int i = 0 ; i < USED_RESOURCES; ++i)
    {
        r->resources[i].mutex.lock = false;
        r->resources[i].mutex.thread = 0x0;
        r->resources[i].pointer = (uint32_t *)&serial_puts;
    }
}

void access_resource(resource_t *res)
{
    _access(&res->mutex);
}

void release_resource(resource_t *res)
{
    _leave(&res->mutex);
}
