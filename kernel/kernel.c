
#include <kernel.h>

#define KSTACKSTART 0x6000
#define KSTACKEXC   0x8000

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

#define KEXP_BOT3 \
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
      __builtin_va_list *va;
      const char *fmt;
      asm("mov r8, sp");
      asm("mov r9, #0x300");
      asm("ldm r9, {%0, %1}" : "=r"(fmt),"=r"(va));
      asm("mov r9, pc");
      asm("add r9, r9, #4");
      _printf(fmt, va);
      asm("tohere:");
      asm("mov sp, r1");
      for(;;);
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
    }

    if(!ks->iswitch)
    {
      for(ks->threadndx = (ks->threadndx + 1) & 0xf; !ks->threads[ks->threadndx].valid; ks->threadndx = (ks->threadndx + 1) & 0xf);
    }
    ks->iswitch = 0;
    kt = &ks->threads[ks->threadndx];
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
    return;
  }

  if(type != IVT_IRQ && type != IVT_FIQ && type != IVT_SWINT)
  {
    // trap
    serial_putc('!');
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
void __attribute__((naked)) k_exphandler_swi_entry() { KEXP_TOPSWI; k_exphandler(lr, IVT_SWINT); KEXP_BOT3; }

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
    delay(1000);
    printf("Thread 1\n");
  }
}

void sample_thread_2()
{
  for(;;)
  {
    delay(500);
    printf("Thread 2\n");
  }
}


void kmain(uint32_t r0, uint32_t r1, uint32_t atags)
{
  // declare as unused for now
  (void) r0;
  (void) r1;
  (void) atags;

  uint32_t *t0mmio;
  uint32_t *picmmio;

  serial_puts("Hello world from kernel!\n");

  allocate_ivt_entry(IVT_RESET, &k_exphandler_reset_entry); // 0x0
  allocate_ivt_entry(IVT_UNDEF, &k_exphandler_undef_entry); // 0x4
  allocate_ivt_entry(IVT_SWINT, &k_exphandler_swi_entry);   // 0x8
  allocate_ivt_entry(IVT_ABRTP, &k_exphandler_abrtp_entry); // 0x0b
  allocate_ivt_entry(IVT_ABRTD, &k_exphandler_abrtd_entry); // 0x10
  allocate_ivt_entry(IVT_IRQ, &k_exphandler_irq_entry);     // 0x14
  allocate_ivt_entry(IVT_FIQ, &k_exphandler_fiq_entry);     // 0x18

  serial_putc('Z');

  arm_cpsrset(arm_cpsrget() & ~(1 << 7)); // enable IRQ
  //arm_cpsrset(arm_cpsrget() & ~(1 << 6));

  init_threading();

  create_thread(&sample_thread_1, 0x100000, 0x110000);
  create_thread(&sample_thread_2, 0x130000, 0x140000);

  init_guarded();

  printf("Hello world\n");
  printf("Hello world\n");

  picmmio = (uint32_t *)0x14000000;
  picmmio[PIC_IRQ_ENABLESET] = (1 << 5) | (1 << 6) | (1 << 7);

  t0mmio = (uint32_t *)0x13000000;
  t0mmio[REG_LOAD] = 0xffffff;
  t0mmio[REG_BGLOAD] = 0xffffff;
  t0mmio[REG_CTRL] = CTRL_ENABLE | CTRL_MODE_PERIODIC | CTRL_SIZE_32 | CTRL_DIV_NONE | CTRL_INT_ENABLE;
  t0mmio[REG_INTCLR] = ~0;

  serial_puts("K\n");
  for(;;);
}
