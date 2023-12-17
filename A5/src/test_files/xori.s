.globl _start
_start:
    li a1, 5
    xori a2, a1, 3

    li a3, 6
    beq a2, a3, exit_program

    li a7, 10
    li a0, 1
    ecall

exit_program:
    ori a7, x0, 3
    ecall