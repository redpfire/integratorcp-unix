
#define IVT_RESET   0x00
#define IVT_UNDEF   0x01
#define IVT_SWINT   0x02
#define IVT_ABRTP   0x03
#define IVT_ABRTD   0x04
#define IVT_RESV1   0x05
#define IVT_IRQ     0x06
#define IVT_FIQ     0x07

#define ARM_MODE_USER   0x10
#define ARM_MODE_FIQ	 0x11
#define ARM_MODE_IRQ	 0x12
#define ARM_MODE_SUPER  0x13
#define ARM_MODE_ABORT	 0x17
#define ARM_MODE_UNDEF  0x1b
#define ARM_MODE_SYS    0x1f
#define ARM_MODE_MON    0x16

#define CTRL_ENABLE			0x80
#define CTRL_MODE_FREE		0x00
#define CTRL_MODE_PERIODIC	0x40
#define CTRL_INT_ENABLE		(1<<5)
#define CTRL_DIV_NONE		0x00
#define CTRL_DIV_16			0x04
#define CTRL_DIV_256		0x08
#define CTRL_SIZE_32		0x02
#define CTRL_ONESHOT		0x01
 
#define REG_LOAD		0x00
#define REG_VALUE		0x01
#define REG_CTRL		0x02
#define REG_INTCLR		0x03
#define REG_INTSTAT		0x04
#define REG_INTMASK		0x05
#define REG_BGLOAD		0x06
 
#define PIC_IRQ_STATUS			0x0
#define PIC_IRQ_RAWSTAT			0x1
#define PIC_IRQ_ENABLESET		0x2
#define PIC_IRQ_ENABLECLR		0x3
#define PIC_INT_SOFTSET			0x4
#define PIC_INT_SOFTCLR			0x5
 
#define PIC_FIQ_STATUS			8
#define PIC_FIQ_RAWSTAT			9
#define PIC_FIQ_ENABLESET		10
#define PIC_FIQ_ENABLECLR		11
