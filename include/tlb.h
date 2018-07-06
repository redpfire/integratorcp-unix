
#include <inttype.h>

/* first level table */
#define TLB_FAULT			0x000		/* entry is unmapped (bits 32:2 can be used for anything) but access generates an ABORT */
#define TLB_SECTION			0x002		/* entry maps 1MB chunk */
#define TLB_COARSE			0x001		/* sub-table */
#define TLB_DOM_NOACCESS	0x00		/* generates fault on access */
#define TLB_DOM_CLIENT		0x01		/* checked against permission bits in TLB entry */
#define TLB_DOM_RESERVED	0x02		/* reserved */
#define TLB_DOM_MANAGER		0x03		/* no permissions */
#define TLB_STDFLAGS		0xc00		/* normal flags */
/* second level coarse table */
#define TLB_C_LARGEPAGE		0x1			/* 64KB page */
#define TLB_C_SMALLPAGE		0x2			/* 4KB page */
/* AP (access permission) flags for coarse table [see page 731 in ARM_ARM] */
#define TLB_C_AP_NOACCESS	(0x00<<4)	/* no access */
#define TLB_C_AP_PRIVACCESS	(0x01<<4)	/* only system access  RWXX */
#define TLB_C_AP_UREADONLY	(0x02<<4)	/* user read only  RWRX */
#define TLB_C_AP_FULLACCESS	(0x03<<4)	/* RWRW */	
/* AP (access permission) flags [see page 709 in ARM_ARM; more listed] */
#define TLB_AP_NOACCESS		(0x00<<10)	/* no access */
#define TLB_AP_PRIVACCESS	(0x01<<10)	/* only system access  RWXX */
#define TLB_AP_UREADONLY	(0x02<<10)	/* user read only  RWRX */
#define TLB_AP_FULLACCESS	(0x03<<10)	/* RWRW */

void arm_tlbset0(uint32_t base);
void arm_tlbset1(uint32_t base);
void arm_tlbsetmode(uint32_t val);
void arm_tlbsetdom(uint32_t val);
uint32_t arm_tlbgetctrl();
void arm_tlbsetctrl(uint32_t ctrl);
