
#include <stddef.h>
#include <thread.h>
#include <inttype.h>
#include <serial.h>
#include <guarded.h>
#include <stdio.h>
#include <mem.h>

#include <kernel.h>

#define KEXP_TOPSWI \
	uint32_t			lr; \
	asm("mov sp, %[ps]" : : [ps]"i" (KSTACKEXC)); \
	asm("push {lr}"); \
	asm("push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12}"); \
	asm("mov %[ps], lr" : [ps]"=r" (lr));

#define KEXP_TOP3 \
	uint32_t			lr; \
	asm("mov sp, %[ps]" : : [ps]"i" (KSTACKEXC)); \
	asm("sub lr, lr, #4"); \
	asm("push {lr}"); \
	asm("push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12}"); \
  asm("mrs r0, spsr"); \
  asm("push {r0}"); \
	asm("mov %[ps], lr" : [ps]"=r" (lr));

#define KEXP_BOTSWI \
  asm("pop {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12}"); \
	asm("LDM sp!, {pc}^");

#define KEXP_BOT3 \
  asm("pop {r0}");\
  asm("msr spsr, r0");\
	asm("pop {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12}"); \
	asm("LDM sp!, {pc}^");

static inline void mmio_w(uint32_t reg, uint32_t data)
{
  *(volatile uint32_t *)reg = data;
}

static inline uint32_t mmio_r(uint32_t reg)
{
  return *(volatile uint32_t *)reg;
}

static inline void delay(uint32_t count)
{
  asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
               : "=r"(count): [count]"0"(count) : "cc");
}

void k_exphandler(uint32_t lr, uint32_t type)
{
  uint32_t *t0mmio;
  uint32_t swi;
  kstate_t *ks;
  thread_t *kt;
  uint32_t			__lr, __sp, __spsr;
  ks = (kstate_t *)KSTATEADDR;

  if(type == IVT_SWINT)
  {
    swi = ((uint32_t *)((uint32_t)lr - 4))[0] & 0xffff;
    if(swi == 4)
    {
      serial_putc('@');
    }
    else if(swi == 1)
    {
      asm("mov r8, sp");
      __builtin_va_list *va;
      const char *fmt;
      asm("mov r6, #0x300");
      asm("ldm r6, {%0, %1}" : "=r"(fmt),"=r"(va));
      asm("mov r7, pc");
      asm("add r7, r7, #4");
      _printf(fmt, va);
      asm("tohere:");
      asm("mov sp, r8");
      return;
    }
  }

  if(type == IVT_IRQ)
  {
    t0mmio = (uint32_t *)0x13000000;
    t0mmio[REG_INTCLR] = 1; // clear interrupt in timer HW
    t0mmio[REG_CTRL] = t0mmio[REG_CTRL] | CTRL_ENABLE;
    if(!ks->iswitch)
    {
      arm_tlbset1((uint32_t)ks->vmmk.table); // switch back to kernel table
      asm("mcr p15, #0, r0, c8, c7, #0");
      kt = &ks->threads[ks->threadndx];
      kt->pc = ((uint32_t *)KSTACKEXC)[-1];
			kt->r12 = ((uint32_t *)KSTACKEXC)[-2];
			kt->r11 = ((uint32_t *)KSTACKEXC)[-3];
			kt->r10 = ((uint32_t *)KSTACKEXC)[-4];
			kt->r9 = ((uint32_t *)KSTACKEXC)[-5];
			kt->r8 = ((uint32_t *)KSTACKEXC)[-6];
			kt->r7 = ((uint32_t *)KSTACKEXC)[-7];
			kt->r6 = ((uint32_t *)KSTACKEXC)[-8];
			kt->r5 = ((uint32_t *)KSTACKEXC)[-9];
			kt->r4 = ((uint32_t *)KSTACKEXC)[-10];
			kt->r3 = ((uint32_t *)KSTACKEXC)[-11];
			kt->r2 = ((uint32_t *)KSTACKEXC)[-12];
			kt->r1 = ((uint32_t *)KSTACKEXC)[-13];
			kt->r0 = ((uint32_t *)KSTACKEXC)[-14];
			kt->cpsr = ((uint32_t *)KSTACKEXC)[-15];

      printk("kt->lr:0x%x threadndx: 0x%x\n", kt->lr, ks->threadndx);

      asm("mrs r0, cpsr\n\
           bic r0, r0, #0x1f\n\
           orr r0, r0, #0x1f\n\
           msr cpsr, r0\n\
           mov %[sp], sp\n\
           mov %[lr], lr\n\
           bic r0, r0, #0x1f\n\
           orr r0, r0, #0x12\n\
           msr cpsr, r0\n\
           " : [sp]"=r"(__sp), [lr]"=r"(__lr));
      kt->sp = __sp;
      kt->lr = __lr;
      printk("<---threadndx:0x%x kt->sp:0x%x kt->pc:0x%x kt->lr:0x%x\n", ks->threadndx, kt->sp, kt->pc, kt->lr);
    }

    if(!ks->iswitch)
    {
      for(ks->threadndx = (ks->threadndx + 1) & 0xf; !ks->threads[ks->threadndx].valid; ks->threadndx = (ks->threadndx + 1) & 0xf);
    }
    ks->iswitch = 0;
    kt = &ks->threads[ks->threadndx];
    // for (int x = 0; x < 16; ++x)
		// 	printk("stack[0x%x]:0x%x\n", x, ((uint32_t *)KSTACKEXC)[-x]);
    ((uint32_t *)KSTACKEXC)[-1] = kt->pc;
		((uint32_t *)KSTACKEXC)[-2] = kt->r12;
 		((uint32_t *)KSTACKEXC)[-3] = kt->r11;
		((uint32_t *)KSTACKEXC)[-4] = kt->r10;
		((uint32_t *)KSTACKEXC)[-5] = kt->r9;
		((uint32_t *)KSTACKEXC)[-6] = kt->r8;
 		((uint32_t *)KSTACKEXC)[-7] = kt->r7;
		((uint32_t *)KSTACKEXC)[-8] = kt->r6;
		((uint32_t *)KSTACKEXC)[-9] = kt->r5;
		((uint32_t *)KSTACKEXC)[-10] = kt->r4;
		((uint32_t *)KSTACKEXC)[-11] = kt->r3;
		((uint32_t *)KSTACKEXC)[-12] = kt->r2;
		((uint32_t *)KSTACKEXC)[-13] = kt->r1;
		((uint32_t *)KSTACKEXC)[-14] = kt->r0;
		((uint32_t *)KSTACKEXC)[-15] = kt->cpsr;
    asm("mrs r0, cpsr\n\
         bic r0, r0, #0x1f\n\
         orr r0, r0, #0x1f\n\
         msr cpsr, r0\n\
         mov sp, %[sp]\n\
         mov lr, %[lr]\n\
         bic r0, r0, #0x1f\n\
         orr r0, r0, #0x12\n\
         msr cpsr, r0\n\
         " :: [sp]"r"(kt->sp), [lr]"r"(kt->lr));
    arm_tlbset1((uint32_t) kt->vmm.table);
    asm("mcr p15, #0, r0, c8, c7, #0");
    uint32_t phy;
    kvmm_getphy(kt->vmm, 0x80000000, &phy);
    printk("--->threadndx:0x%x kt->sp:0x%x kt->pc:0x%x kt->lr:0x%x real:%x\n", ks->threadndx, kt->sp, kt->pc, kt->lr, phy);
    return;
  }

  if(type != IVT_IRQ && type != IVT_FIQ && type != IVT_SWINT)
  {
    printk("Trap! %x", type);
    // trap
    for(;;);
  }
  return;
}

void __attribute__((naked)) k_exphandler_irq_entry() { KEXP_TOP3; k_exphandler(lr, IVT_IRQ); KEXP_BOT3; }
void __attribute__((naked)) k_exphandler_fiq_entry() { KEXP_TOP3; k_exphandler(lr, IVT_FIQ); KEXP_BOT3; }
void __attribute__((naked)) k_exphandler_reset_entry() { KEXP_TOP3; k_exphandler(lr, IVT_RESET); KEXP_BOT3; }
void __attribute__((naked)) k_exphandler_undef_entry() { KEXP_TOP3; k_exphandler(lr, IVT_UNDEF); KEXP_BOT3; }
void __attribute__((naked)) k_exphandler_abrtp_entry() { KEXP_TOP3; k_exphandler(lr, IVT_ABRTP); KEXP_BOT3; }
void __attribute__((naked)) k_exphandler_abrtd_entry() { KEXP_TOP3; k_exphandler(lr, IVT_ABRTD); KEXP_BOT3; }
void __attribute__((naked)) k_exphandler_swi_entry() { KEXP_TOPSWI; k_exphandler(lr, IVT_SWINT); KEXP_BOTSWI; }

void allocate_ivt_entry(uint8_t intr, void *addr)
{
  uint32_t x;
  uint32_t *y = (uint32_t *)0x0;
  y += intr;
  x = 0xEA000000 | (((uint32_t)addr - (8 + (4 * intr))) >> 2);
  asm("str %0, [%1]" ::"r"(x),"r"(y));
}

void raw_mem_w(uint32_t addr, uint32_t val)
{
  asm("str %0, [%1]" :: "r"(val),"r"(addr));
}

void arm_cpsrset(uint32_t r)
{
  asm("msr cpsr, %[ps]" :: [ps]"r"(r));
}

uint32_t arm_cpsrget()
{
  uint32_t r;
  asm("mrs %[ps], cpsr" : [ps]"=r"(r));
  return r;
}

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif

void sample_thread_1()
{
  for(;;)
  {
    uint32_t a = 0xa0000000;
    asm("mov r0, #0x41");
    asm("mov r1, %0" :: "r"(a));
    asm("str r0, [r1]");
  }
}

void sample_thread_2()
{
  for(;;)
  {
    uint32_t a = 0xa0000000;
    asm("mov r0, #0x42");
    asm("mov r1, %0" :: "r"(a));
    asm("str r0, [r1]");
  }
}

void debugger()
{
  char c;
  printk("(enter) ");
  while( (c = serial_getc()) != '\r' );
  serial_putc('\n');
}

void kmain(uint32_t r0, uint32_t r1, uint32_t atags)
{
  // declare as unused for now
  (void) r0;
  (void) r1;
  (void) atags;

  uint32_t *t0mmio;
  uint32_t *picmmio;

  printk("Opening a portal to narnia.\n");
  //arm_cpsrset(arm_cpsrget() & ~(1 << 6));

  init_threading();

  debugger();

  allocate_ivt_entry(IVT_RESET, &k_exphandler_reset_entry); // 0x0
  allocate_ivt_entry(IVT_UNDEF, &k_exphandler_undef_entry); // 0x4
  allocate_ivt_entry(IVT_SWINT, &k_exphandler_swi_entry);   // 0x8
  allocate_ivt_entry(IVT_ABRTP, &k_exphandler_abrtp_entry); // 0x0b
  allocate_ivt_entry(IVT_ABRTD, &k_exphandler_abrtd_entry); // 0x10
  allocate_ivt_entry(IVT_IRQ, &k_exphandler_irq_entry);     // 0x14
  allocate_ivt_entry(IVT_FIQ, &k_exphandler_fiq_entry);     // 0x18

  printk("[IRQ] Allocated all entries.\n");

  create_thread(&sample_thread_1);
  create_thread(&sample_thread_2);

  init_guarded();

  arm_cpsrset(arm_cpsrget() & ~(1 << 7)); // enable IRQ

  picmmio = (uint32_t *)0x14000000;
  picmmio[PIC_IRQ_ENABLESET] = (1 << 5) | (1 << 6) | (1 << 7);

  t0mmio = (uint32_t *)0x13000000;
  t0mmio[REG_LOAD] = 0x0fffff;
  t0mmio[REG_BGLOAD] = 0x0fffff;
  t0mmio[REG_CTRL] = CTRL_ENABLE | CTRL_MODE_PERIODIC | CTRL_SIZE_32 | CTRL_DIV_NONE | CTRL_INT_ENABLE;
  t0mmio[REG_INTCLR] = ~0;

  serial_puts("K\n");
  for(;;);
}
