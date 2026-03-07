@ This file contains the end block that completes the metadata loop
@ required by the RP2350 ROM bootloader to verify the executable.

.syntax unified
.cpu cortex-m33                                     @ RP2350 uses ARM Cortex-M33              
.thumb

.section .embedded_end_block, "a"
.p2align 2                                          @ ROM bootloader can only read blocks if they are 4-byte aligned. This makes sure that next instruction or data 
                                                    @ gets put on the next memory address that is divisable by 4
.global embedded_end_block                    
embedded_end_block:
    .word 0xffffded3                                @ Block start marker - ROM bootloader searches for this value in flash to find a valid block                                
    .byte 0xfe                                      @ Item type - IGNORED: The ROM bootloader will ignores this item but it is used as a placeholder since there always needs to be one item before LAST item. 
    .byte 0x01                                      @ Size of IGNORED (= 1)
    .hword 0x0000                                   @ The data of IGNORED. Values does not matter since ROM bootloader ignores it
    
    .byte 0xff                                      @ Item type - LAST: Tells the ROM bootloader that item list ends here, there are no more items. The hi- and lo-byte are used to make sure that no items are missed.
                                                    @ In this case the value of the bytes combined is 1 since there is only one item (IGNORED).
    .byte 0x01                                      @ Total item size lo-byte  \  together a 16-bit
    .byte 0x00                                      @ Total item size hi-byte  /  value = 0x0001
    .byte 0x00                                      @ Padding to hold 4-byte alignment
    
    .word embedded_block - embedded_end_block       @ Reversed logic from start_block. This tells the ROM bootloader where the start_block is so it can jump to it.
    .word 0xab123579                                @ Block end marker - ROM bootloader verifies that the block ends here.
embedded_end_block_end: