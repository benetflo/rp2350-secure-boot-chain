.syntax unified
.cpu cortex-m33                                         @ RP2350 uses ARM Cortex-M33
.thumb

.section .vectors, "ax"
.align 2
.global __VECTOR_TABLE
__VECTOR_TABLE:
    .word __StackTop
    .word _reset_handler
    .word _default_handler
    .word _default_handler
    .word _default_handler
    .word _default_handler
    .word _default_handler
    .word _default_handler
    .word 0
    .word 0
    .word 0
    .word _default_handler
    .word _default_handler
    .word 0
    .word _default_handler
    .word _default_handler

.section .text
.thumb_func
.global _default_handler
_default_handler:
    bkpt #0
    b _default_handler

.thumb_func
.global _reset_handler
_reset_handler:
    ldr r1, =__etext
    ldr r2, =__data_start__
    ldr r3, =__data_end__
    b 1f
0:  ldm r1!, {r0}
    stm r2!, {r0}
1:  cmp r2, r3
    blo 0b

    ldr r1, =__bss_start__
    ldr r2, =__bss_end__
    movs r0, #0
    b 3f
2:  stm r1!, {r0}
3:  cmp r1, r2
    blo 2b

    bl main
_halt:
    bkpt #0
    b _halt