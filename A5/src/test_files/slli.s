.globl _start
_start:
    li a1, 5
    slli a2, a1, 3  

    # Exit program
	ori	a7,x0,3
    ecall