  .syntax unified
  .cpu cortex-m4
  .fpu softvfp
  .thumb

.global  g_pfnVectors

  .section  .text.Reset_Handler
  .weak  Reset_Handler
  .type  Reset_Handler, %function
Reset_Handler:  

  movs  r1, #0
  b  LoopCopyDataInit
CopyDataInit:
  ldr  r3, =_sidata
  ldr  r3, [r3, r1]
  str  r3, [r0, r1]
  adds  r1, r1, #4
LoopCopyDataInit:
  ldr  r0, =_sdata
  ldr  r3, =_edata
  adds  r2, r0, r1
  cmp  r2, r3
  bcc  CopyDataInit

  ldr  r2, =_sbss
  b  LoopFillZerobss
FillZerobss:
  movs  r3, #0
  str  r3, [r2], #4
LoopFillZerobss:
  ldr  r3, = _ebss
  cmp  r2, r3
  bcc  FillZerobss

  push {r4,lr}
  bl __libc_init_array
  pop {r4,lr}

  b  init
.size  Reset_Handler, .-Reset_Handler

   .section  .isr_vector,"a",%progbits
  .type  g_pfnVectors, %object
g_pfnVectors:
  .word  0x12345678;		/* magic number */
  .word  Reset_Handler		/* reg() function */
  .word  __libc_fini_array	/* dereg() function */
  .word  _eflash                /* end of flash */
  .word  _sbss			/* end of ram */
  .size  g_pfnVectors, .-g_pfnVectors
