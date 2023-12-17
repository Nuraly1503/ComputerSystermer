.globl _start
_start:
    .data
word:
    .text
    la a1, word  
    li a2, 0x484F
    sh a2, 0(a1)

    # Exit program
    ori a7, x0, 3
    ecall