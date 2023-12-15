        .globl _start
_start:

  # Testing imm_i
  addi  a0, x0, 20
  addi  a0, a0, 34
  addi  a7, x0, 2     # print '6'
  ecall

  # Testing imm_s
  addi  a4, x0, 53        # '5'
  lui	  a5, 0   
  sb    a4, 10(a5)     # store in memory address 0xa
  lb    a0, 10(a5)     
  addi  a7, x0, 2         # print '5'
  ecall 

  # Testing imm_b
  addi  a0, x0, 104   # 'h'
  addi  a1, x0, 104   # 'h'
  beq   a0, a1, BEQ_jump  # jump to Branch
  addi   a0, x0, 48
  addi  a7, x0, 2         # print '0'
  ecall  
  BEQ_jump:
  addi  a7, x0, 2         # print 'h'
  ecall 

  # Testing imm_u
  lui   a0, 55        # '7' char, 00110111, testing imm_i
  srli  a0, a0, 12    # shift left 12 bits, testing shamt
  addi  a7, x0, 2     # print '7'
  ecall

  # Testing imm_j
  # TODO
  jal JAL_jump      # jump
  addi a0, x0, 65   # 'A'
  addi a7, x0, 2
  ecall
  addi a0, x0, 76   # 'L'
  addi a7, x0, 2
  ecall
  jal TERMINATE

  JAL_jump:
  addi a0, x0, 74   # 'J'
  addi a7, x0, 2
  ecall
  ret

  # Terminate 
  TERMINATE:              
  addi	a7,x0,3
  ecall

  
