
#include <tlb.h>

#define KVMM_SUCCESS		1
#define KVMM_FAILURE		0
#define KVMM_UNMAP			0x80000000
#define KVMM_USER			0x40000000
#define KVMM_KERNEL			0x20000000

void *kkpalloc();
void *kupalloc();
int kvmm_init(k_vmm_t *table);
int kvmm_map(k_vmm_t table, uint32_t virt, uint32_t physical, uint32_t count, uint32_t flags);
int kvmm_getunusedregion(k_vmm_t table, uint32_t *virt, uint32_t count, uint32_t lowerLimit, uint32_t upperLimit);
int kvmm_getphy(k_vmm_t table, uint32_t virt, uint32_t *out);
int kvmm_allocregion(k_vmm_t table, uint32_t virt, uint32_t count, uint32_t flags);
int kvmm_freeregion(k_vmm_t table, uint32_t virt, uint32_t count);
