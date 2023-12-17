.globl _start
_start:
    li a1, 5
    slti a2, a1, 6

    # Exit program
    ori a7, x0, 3
    ecall