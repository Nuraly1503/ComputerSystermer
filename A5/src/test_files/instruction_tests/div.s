.globl _start
_start:
    li a3, 10
    li a2, 100
    div a4, a2, a3
    ori a7, x0, 3
    ecall