/* This linker script does the following:
 * 1. Defines where flash and RAM start and how much space they take up.
 * 2. Defines _reset_handler as the first function to run at startup.
 * 3. Places the vector table, code and IMAGE_DEF metadata in flash.
 * 4. __etext marks where .data is stored in flash so crt0.S knows where to copy it from.
 * 5. Places .data and .bss in RAM.
 * 6. Places the stack at the top of RAM where it grows downward.
 */


MEMORY {
    FLASH(rx)  : ORIGIN = 0x10000000, LENGTH = 256K     /* Flash start address and size. rx = read + execute */
    RAM(rwx)   : ORIGIN = 0x20000000, LENGTH = 520K     /* RAM start address and size. rwx = read + write + execute */
}

ENTRY(_reset_handler)                               /* First function to run at startup */

SECTIONS {
    .text : {
        KEEP(*(.vectors))                           /* Vector table must be first in flash */
        *(.text*)                                   /* All code (assembly and C code for the bootloader)*/
        *(.rodata*)                                 /* Read-only data (constants) */
    } > FLASH                                       /* Place in flash */

    .embedded_block : {
        KEEP(*(.embedded_block))                    /* IMAGE_DEF start block - required by ROM bootloader */
    } > FLASH

    .embedded_end_block : {
        KEEP(*(.embedded_end_block))                /* IMAGE_DEF end block - completes the metadata loop */
    } > FLASH

    __etext = .;                        

    .data : {
        __data_start__ = .;                         /* Start of .data in RAM */
        *(.data*)                                   /* Global variables with initial values */
        __data_end__ = .;                           /* End of .data in RAM */
    } > RAM AT > FLASH                              /* Lives in RAM but stored in flash - copied by crt0.S at startup */

    .bss : {
        __bss_start__ = .;                          /* Start of .bss in RAM */
        *(.bss*)                                    /* Global variables without initial values - zeroed by crt0.S at startup */
        *(COMMON)                                   /* Uninitialized global variables */
        __bss_end__ = .;                            /* End of .bss in RAM */
    } > RAM                                         /* Only in RAM, nothing stored in flash */

    __StackTop = ORIGIN(RAM) + LENGTH(RAM);         /* Stack starts at top of RAM and grows downward */
}