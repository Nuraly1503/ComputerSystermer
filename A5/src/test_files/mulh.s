.globl _start
_start:
    li a3, 0x10000
    li a2, 0x10000
    mulh a4, a2, a3
    ori a7, x0, 3
    ecall