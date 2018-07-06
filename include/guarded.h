
#define GUARDED_POINTER 0x1850
#define USED_RESOURCES 1

#define G_SERIAL 0

#include <inttype.h>
#include <spinlock.h>

typedef struct
{
    mutex_t mutex;
    uint32_t *pointer;
} resource_t;

typedef struct
{
    resource_t resources[64];
} resources_t;

void init_guarded();
void access_resource(resource_t*);
void release_resource(resource_t*);
