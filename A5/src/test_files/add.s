        .globl _start
_start:
	li a3, 100
	li x2, 50
	add	a4, x2, a3
	ori	a7,x0,3
	ecall
