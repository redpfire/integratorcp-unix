
.section ".text.kickstart"

.globl _start

// Entry point for the kernel.
// r15 -> should begin execution at 0x8000.
// r0 -> 0x00000000
// r1 -> 0x00000C42
// r2 -> 0x00000100 - start of ATAGS
// preserve these registers as argument for kernel_main

_start:
  mov sp, #0x2000

  // clear out bss
  ldr r4, =__bss_start
  ldr r9, =__bss_end
  mov r5, #0
  mov r6, #0
  mov r7, #0
  mov r8, #0
  b 2f

1:
  stmia r4!, {r5-r8}

2:
  cmp r4, r9
  blo 1b

  ldr r3, =kmain
  blx r3

halt:
  wfe
  b halt
