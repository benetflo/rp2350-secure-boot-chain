.syntax unified
.cpu cortex-m33                                         @ RP2350 uses ARM Cortex-M33
.thumb

.section .vectors, "ax"                                 @ create a section called .vectors that stores the vector table. 
                                                        @ The vector table is a bunch of hard coded addresses that the linker puts in flash when building.
                                                        @ The vector table values are either addresses in RAM or flash.
.align 2                                                @ 4 byte alignment
.global __VECTOR_TABLE                                  @ make it visable to linker
__VECTOR_TABLE:
    .word __StackTop                                    @ slot 0:  stack pointer value
    .word _reset_handler                                @ slot 1:  reset handler - where CPU jumps at startup
    .word _default_handler                              @ slot 2:  NMI handler - Example: power failure detected by hardware
    .word _default_handler                              @ slot 3:  HardFault handler - Critical error in CPU. Example: division by zero
    .word _default_handler                              @ slot 4:  MemManage handler - Unvalid memory access. Example:  reading from an address outside allowed memory region
    .word _default_handler                              @ slot 5:  BusFault handler - Error at memory transfer. Example: reading from an address that does not exist in memory
    .word _default_handler                              @ slot 6:  UsageFault handler - Unvalid instruction. Example: executing an instruction the CPU does not support
    .word _default_handler                              @ slot 7:  SecureFault handler - Security violation e.g. Non-Secure code accessing Secure memory (TrustZone). Example: Non-Secure code tries to read a Secure variable
    .word 0                                             @ slot 8:  reserved
    .word 0                                             @ slot 9:  reserved
    .word 0                                             @ slot 10: reserved
    .word _default_handler                              @ slot 11: SVCall handler - System calls from software. Example: an RTOS calls OS services
    .word _default_handler                              @ slot 12: DebugMonitor handler - Debugger event. Example: breakpoint triggered via debugger
    .word 0                                             @ slot 13: reserved
    .word _default_handler                              @ slot 14: PendSV handler - Low priority system call. Example: RTOS switches context between tasks
    .word _default_handler                              @ slot 15: SysTick handler - Periodic timer interrupt. Example: timer ticks every 1ms to keep track of time

.section .text
.thumb_func
.global _default_handler                                @ fallback handler for all unexpected interrupts and faults.
                                                        @ stops the CPU in a controlled way instead of crashing randomly.
                                                        @ all default handlers in vector table points to this.
_default_handler:
    bkpt #0                                             @ Breakpoint - Signals the debugger to stop
                                                        @ If there is no debugger - HardFault triggers
    b _default_handler                                  @ Jump to self - infinite loop.

.thumb_func
.global _reset_handler
_reset_handler:                                         @ Copies .data section (global variables...)from flash to RAM otherwise they cannot be changed.
    ldr r1, =__etext                                    @ Points to where .data section is stored in flash
    ldr r2, =__data_start__                             @ Points to where .data section should be stored in RAM (start)
    ldr r3, =__data_end__                               @ Points to where .data sections should be stored in RAM (end)
    b 1f
0:  ldm r1!, {r0}
    stm r2!, {r0}
1:  cmp r2, r3
    blo 0b
                                                        @ This block will nollställa global variables in .bss. This is becuase flash contains trash values.
    ldr r1, =__bss_start__                              @ r1 points to start of .bss in RAM
    ldr r2, =__bss_end__                                @ r2 points to end of .bss in RAM
    movs r0, #0                                         @ r0 = 0 (this is what we will write)
    b 3f                                                @ 3f == label 3 forward. This jumps to compare instruction first, so that we don't write a 0 if .bss is empty.
2:  stm r1!, {r0}                                       @ write 0 to RAM, move r1 forward
3:  cmp r1, r2                                          @ are we at the end of .bss yet?
    blo 2b                                              @ no - go back to label 2 and write 4 more zeros
                                                        @ yes - continue to main()

    bl main                                             @ jump to main program
_halt:
    bkpt #0                                             @ main should never return - there is no OS to return to.
                                                        @ if it does return, stop here instead of executing random memory.
    b _halt                                             @ else infinite loop


@ bkpt #0 == breakpoint. CPU instruction that pauses execution.
@ With debugger: debugger catches it and pauses so you can inspect what happened.
@ Without debugger: triggers a HardFault which leads to _default_handler.