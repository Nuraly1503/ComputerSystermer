.globl _start
_start:
    .data
word:
    .byte 0x48, 0x4F, 0x4C, 0x41 #Hex for HOLA

    .text
    la a1, word  
    lh a2, 0(a1)

    # Exit program
    ori a7, x0, 3
    ecall