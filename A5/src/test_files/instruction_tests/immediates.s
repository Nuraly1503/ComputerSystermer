        .globl _start
_start:

  # Testing imm_i
  addi  a0, x0, 40
  addi  a0, a0, 33        # input 'I'
  addi  a7, x0, 2       
  ecall                   # output 'I'

  # Testing imm_s
  addi  a4, x0, 83        # input 'S'
  lui	  a5, 0   
  sb    a4, 10(a5)        # store in memory address 0xa
  lb    a0, 10(a5)     
  addi  a7, x0, 2       
  ecall                   # output 'S'

  # Testing imm_b
  addi  a0, x0, 66        # input 'B'
  addi  a1, x0, 66        # input 'B'
  beq   a0, a1, BEQ_jump  # jump to Branch
  addi   a0, x0, 48
  addi  a7, x0, 2         # output '0'
  ecall  
  BEQ_jump:
  addi  a7, x0, 2         
  ecall                   # output 'B'

  # Testing imm_u
  lui   a0, 85            # input 'U'
  srli  a0, a0, 12        # shift left 12 bits, testing shamt
  addi  a7, x0, 2     
  ecall                   # output 'U'

  # Testing imm_j
  # TODO
  addi a0, x0, 74         # input '0'
  jal JAL_jump            # jump
  addi a7, x0, 2    
  ecall                   # output 'J'
  jal TERMINATE

  JAL_jump:
  addi a0, x0, 74         # input 'J'
  ret

  # Terminate 
  TERMINATE:              
  addi	a7,x0,3
  ecall

  
