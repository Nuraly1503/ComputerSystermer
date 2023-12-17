.globl _start
_start:
    li a3, 5
    li a2, 5
    bne a3, a2, end

end:
    ori a7, x0, 3
    ecall