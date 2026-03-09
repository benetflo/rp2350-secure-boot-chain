@ This file contains metadata about the built binary that the
@ RP2350 ROM bootloader needs to verify the executable.

.syntax unified
.cpu cortex-m33                                     @ RP2350 uses ARM Cortex-M33                
.thumb

.section .embedded_block, "a"
.p2align 2                                          @ ROM bootloader can only read blocks if they are 4-byte aligned. This makes sure that next instruction or data 
                                                    @ gets put on the next memory address that is divisable by 4
.global embedded_block
embedded_block:
    .word 0xffffded3                                @ Block start marker - ROM bootloader searches for this value in flash to find a valid block
    .byte 0x42                                      @ Item type - IMAGE_TYPE: Tells the ROM bootloader that the next data describes the type of the binary 
    .byte 0x01                                      @ Size of IMAGE_TYPE item (= 1)
    .hword 0x1021                                   @ The data of IMAGE_TYPE.
                                                    @ 0x0001 = EXE
                                                    @ 0x0020 = Secure (in ARM Secure-mode)
                                                    @ 0x0000 = ARM (NOT RISC-V)
                                                    @ 0x1000 = RP2350 (NOT RP2040)
                                            
    .byte 0xff                                      @ Item type - LAST: Tells the ROM bootloader that item list ends here, there are no more items. The hi- and lo-byte are used to make sure that no items are missed.
                                                    @ In this case the value of the bytes combined is 1 since there is only one item (IMAGE_TYPE).

    .byte 0x01                                      @ Total item size lo-byte  \  together a 16-bit
    .byte 0x00                                      @ Total item size hi-byte  /  value = 0x0001
    
    .byte 0x00                                      @ Padding to hold 4-byte alignment
    .word embedded_end_block - embedded_block       @ Tells the ROM bootloader where it can find the end_block. Same principle as a linked list, start_block points to end_block.
    .word 0xab123579                                @ Block end marker - ROM bootloader verifies that the block ends here.
embedded_block_end: