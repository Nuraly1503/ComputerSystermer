.globl _start
_start:
    .data
word:
    .word 0x414C4F48
    .text
    la a1, word  
    lw a2, 0(a1)

    # Exit program
    ori a7, x0, 3
    ecall