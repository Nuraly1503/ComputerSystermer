.globl _start
_start:
    li a3, 8
    li a2, 2
    srl a4, a3, a2

    ori a7, x0, 3
    ecall