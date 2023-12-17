        .globl _start
_start:
	li a3, 50
	li x2, 100
	sub	a4, x2, a3
	ori	a7,x0,3
	ecall
