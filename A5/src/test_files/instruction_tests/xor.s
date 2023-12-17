.globl _start
_start:
    li a2, 10
    li a1, 3
    xor a2, a2, a1

exit_program:
    ori a7, x0, 3
    ecall