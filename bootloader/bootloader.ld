MEMORY {
    FLASH(rx)  : ORIGIN = 0x10000000, LENGTH = 256K
    RAM(rwx)   : ORIGIN = 0x20000000, LENGTH = 520K
}

ENTRY(_reset_handler)

SECTIONS {
    .text : {
        KEEP(*(.vectors))
        *(.text*)
        *(.rodata*)
    } > FLASH

    .embedded_block : {
        KEEP(*(.embedded_block))
    } > FLASH

    .embedded_end_block : {
        KEEP(*(.embedded_end_block))
    } > FLASH

    __etext = .;

    .data : {
        __data_start__ = .;
        *(.data*)
        __data_end__ = .;
    } > RAM AT > FLASH

    .bss : {
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        __bss_end__ = .;
    } > RAM

    __StackTop = ORIGIN(RAM) + LENGTH(RAM);
}