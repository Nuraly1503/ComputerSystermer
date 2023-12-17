.globl _start
_start:
    li a3, 100     
    li a2, 2      
    mul a4, a2, a3
    ori a7, x0, 3
    ecall