.globl _start
_start:
    li a1, 5
    andi a2, a1, 3

    # Exit program
    li a7, 10
    ecall