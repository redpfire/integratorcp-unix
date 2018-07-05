
#include <tlb.h>

void arm_tlbset0(uint32_t base) 
{
	asm("mcr p15, 0, %[tlb], c2, c0, 0" : : [tlb]"r" (base));
}
 
void arm_tlbset1(uint32_t base) 
{
	asm("mcr p15, 0, %[tlb], c2, c0, 1" : : [tlb]"r" (base));
}
 
void arm_tlbsetmode(uint32_t val) 
{
	asm("mcr p15, 0, %[tlb], c2, c0, 2" : : [tlb]"r" (val));
}
 
void arm_tlbsetdom(uint32_t val) 
{
	asm("mcr p15, 0, %[val], c3, c0, 0" : : [val]"r" (val));
}
 
uint32_t arm_tlbgetctrl() 
{
	uint32_t			ctrl;
	asm("mrc p15, 0, r0, c1, c0, 0 \n\
	     mov %[ctrl], r0" : [ctrl]"=r" (ctrl));
	return ctrl;
}
 
void arm_tlbsetctrl(uint32_t ctrl) 
{
	asm("mcr p15, 0, %[ctrl], c1, c0, 0" : : [ctrl]"r" (ctrl));
}
