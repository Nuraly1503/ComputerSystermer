# slt.s
.globl _start
_start:
    li a3, 2
    li a2, 3
    slt a4, a3, a2

    # Exit program
    ori a7, x0, 3
    ecall