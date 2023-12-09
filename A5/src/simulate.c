#include "memory.h"
#include "assembly.h"
#include "simulate.h"
#include "decode_helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Ecall
void ecall(RiscvRegister_t* rscv_reg) {
  uint32_t var = rscv_reg->rg[a7];
  switch(var) {
    case 1:
      // returner "getchar()" i A0
      printf("getchar()\n");
      rscv_reg->rg[a0] = getchar();
      break;
    case 2:
      // udfør "putchar(c)", hvor c tages fra A0
      printf("putchar()\n");
      putchar(rscv_reg->rg[a0]);
      break;
    case 3:
    case 93:
      // afslut simulationen
      break;
    default:
      break;
  }
}


/* 
 * Immidiate Encoding Variants helper functions 
 */

// 1100011
void type_B (uint32_t word, RiscvRegister_t* rscv_reg ) 
{
  uint32_t rs1;
  uint32_t rs2;
  // uint32_t rd;
  uint32_t imm;

  rs1 = get_rs1(word);
  rs2 = get_rs2(word);
  // rd = get_rd(word);
  imm = get_imm_B(word);

  uint32_t funct3 = get_funct3(word);

  switch(funct3)
  {
    case 0:
      printf("BEQ\n");
      if (rscv_reg->rg[rs1] == rscv_reg->rg[rs2])
      {
        rscv_reg->PC = rscv_reg->PC + imm;
      }
      break;
    case 1:
      printf("BNE\n");
      if (rscv_reg->rg[rs1] != rscv_reg->rg[rs2])
      {
        rscv_reg->PC = rscv_reg->PC + imm;
      }
      break;
      // Branch less tha Signed
    case 4:
      printf("BLT\n");
      if ((int32_t)rscv_reg->rg[rs1] < (int32_t)rscv_reg->rg[rs2])
      {
        rscv_reg->PC = rscv_reg->PC + imm;
      }
      break;
    case 5:
      // Branch Greater that or equal Signed
      printf("BGE\n");
      if ((int32_t)rscv_reg->rg[rs1] >= (int32_t)rscv_reg->rg[rs2])
      {
        rscv_reg->PC = rscv_reg->PC + imm;
      }
      break;
    case 6:
      // Branch less than unsigned
      printf("BLTU\n");
      if (rscv_reg->rg[rs1] < rscv_reg->rg[rs2])
      {
        rscv_reg->PC = rscv_reg->PC + imm;
      }
      break;
    case 7:
      // Branch greater than or equal unsigned
      printf("BGEU\n");
      if (rscv_reg->rg[rs1] >= rscv_reg->rg[rs2])
      {
        rscv_reg->PC = rscv_reg->PC + imm;
      }
      break;
  };
}

// 0000011
void type_I (uint32_t word, RiscvRegister_t* rscv_reg ) 
{
  uint32_t rs1;
  // uint32_t rs2;
  uint32_t rd;
  uint32_t imm;

  rs1 = get_rs1(word);
  // rs2 = get_rs2(word);
  rd = get_rd(word);
  imm = get_imm_I(word);

  uint32_t funct3 = get_funct3(word);

  switch (funct3)
  {
  case 0:
    // Load Byte
    printf("LB\n");
    rscv_reg->rg[rd] = rscv_reg->rg[rs1] + imm;
    break;
  case 1:
    printf("LH\n");
    break;
  case 2:
    printf("LW\n");
    break;
  case 4:
    printf("LBU\n");
    break;
  case 5:
    printf("LHU\n");
    break;
  }
}


//0100011
void type_S (uint32_t word, RiscvRegister_t* rscv_reg) 
{
  uint32_t funct3;
  uint32_t rs1;
  uint32_t rs2;
  // uint32_t rd;
  uint32_t imm;
  int8_t byte;
  int16_t halfword;
  
  
  
  rs1 = get_rs1(word);
  rs2 = get_rs2(word);
  // rd = get_rd(word);
  imm = get_imm_S(word);
  byte = (int8_t)rscv_reg->rg[rs2];
  halfword = (int16_t) rscv_reg->rg[rs2];


  funct3 = get_funct3(word);

  switch (funct3)
  {
  case 0:
    // Store byte in memory
    printf("SB\n");
    // Pointer to calculateds adress
    int8_t *bp = (int8_t *) (rscv_reg->rg[rs1]+ imm);
    // Store byte in memory
    *bp = byte;
    break;
  case 1:
    // store halfword in memory 
    printf("SH\n");
    // Pointer for halfword
    int16_t *hp = (int16_t *) (rscv_reg->rg[rs1]+ imm);
    // Store halfword in memory
    *hp = halfword;
    break;
  case 2:
    // store word in memory 
    printf("SW\n");
    // Pointer for word
    int32_t *wp = (int32_t *) (rscv_reg->rg[rs1]+ imm);
    // stpre word in memory
    *wp = rscv_reg->rg[rs2];
    break;
  }
}

//0010011
void type_I2 (uint32_t word, RiscvRegister_t* rscv_reg)
{
  uint32_t funct3;
  uint32_t rs1;
  uint32_t rd;
  uint32_t imm;
  uint32_t funct7;

  funct7 = get_funct7(word);
  funct3 = get_funct3(word);
  rd = get_rd(word);
  rs1 = get_rs1(word);
  imm = get_imm_I(word);

  // Debug
  printf("rs1: %u\n", rs1);
  printf("imm11: %i\n", imm);
  printf("rd: %u\n", rd);

  switch (funct3)
  {
  case 0:
    printf("ADDI\n");
      rscv_reg->rg[rd] = imm + rscv_reg->rg[rs1];
      printf("reg[rs1]: %lli\n", rscv_reg->rg[rd]);
    break;
  case 2:
    // Set less than immediate signed
    printf("SLTI\n");
    if (rscv_reg->rg[rs1] < imm)
    {
      rscv_reg->rg[rd] = 1;
    }
    else
    {
      rscv_reg->rg[rd] = 0;
    }
    break;
  case 3:
    // Set less than immediate unsigned
    printf("SLTIU\n");
    if (rscv_reg->rg[rs1] < imm)
    {
      rscv_reg->rg[rd] = 1;
    }
    else
    {
      rscv_reg->rg[rd] = 0;
    }
    break;
  case 4:
    // Exclusive or immediate
    printf("XORI\n");
    rscv_reg->rg[rd] = rscv_reg->rg[rs1] ^ imm;
    break;
  case 6:
    printf("ORI\n");
    rscv_reg->rg[rd] = imm | rscv_reg->rg[rs1];
    printf("rg[rd]: %lli\n", rscv_reg->rg[rd]);
    break;
  case 7:
    printf("ANDI\n");
    rscv_reg->rg[rd] = rscv_reg->rg[rs1] & imm;
    break;
  
  // Next ones are different types with the shmat
  
  case 1:
    // Shift left immidiate
    printf("SLLI\n");
    rscv_reg->rg[rd] = rscv_reg->rg[rs1] << imm;
    break;
  case 5:
    printf("SRLI/SRAI\n");            //OBS
    if (funct7 == 0){
      printf("SRLI\n");
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] >> imm;
    }
    else
    {
      printf("SRAI\n");
      rscv_reg->rg[rd] = (int32_t)rscv_reg->rg[rs1] >> imm;
    }
    break;
  }
}

//0110011
void type_R (uint32_t word, RiscvRegister_t* rscv_reg ) 
{
  uint32_t rs1;
  uint32_t rs2;
  uint32_t rd;


  rs2 = get_rs2(word);
  rd = get_rd(word);
  rs1 = get_rs1(word);
  uint32_t funct3 = get_funct3(word);
  uint32_t funct7 = get_funct7(word);

  switch (funct3)
  {
  case 0:
    printf("ADD/SUB\n");               /// OBS 
    if (funct7 == 0)
    {
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] + rscv_reg->rg[rs2];
    }
    else
    {
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] - rscv_reg->rg[rs2];
    }
    printf("ADD rg[rd]: %lli\n", rscv_reg->rg[rd]);
    break;
  case 1:
    printf("SLL\n");
    rscv_reg->rg[rd] = rscv_reg->rg[rs1] << rscv_reg->rg[rs2];
   printf("rg[rd]: %lli\n", rscv_reg->rg[rd]);
    break;
  case 2:
    // set less than if rs1 < rs2 else 0
    printf("SLT\n");
    if ((int32_t) rscv_reg->rg[rs1] < (int32_t)rscv_reg->rg[rs2]) {
      rscv_reg->rg[rd] = 1;
    } else {
      rscv_reg->rg[rd] = 0;
    }
    break;
  case 3:
    // Set less unsigned cast to unsigned
    printf("SLTU\n");
    if (rscv_reg->rg[rs1] < rscv_reg->rg[rs2]) {
      rscv_reg->rg[rd] = 1;
    } else {
      rscv_reg->rg[rd] = 0;
    }
    printf("rg[rd]: %lli\n", rscv_reg->rg[rd]);
    break;
  case 4:
    // Exclusive or
    printf("XOR\n");
    rscv_reg->rg[rd] = rscv_reg->rg[rs1]^rscv_reg->rg[rs2];
    break;
  case 5:
    printf("SRL/SRA\n");         /// OBS
    if (funct7 == 0)
    {
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] >> rscv_reg->rg[rs2];
    }
    else
    {
      rscv_reg->rg[rd] = (int32_t)rscv_reg->rg[rs1] >> rscv_reg->rg[rs2];
    }
    break;
  case 6:
    printf("OR\n");
    rscv_reg->rg[rd] = rscv_reg->rg[rs1] | rscv_reg->rg[rs2];
    break;
  case 7:
    printf("AND\n");
    rscv_reg->rg[rd] = rscv_reg->rg[rs1] & rscv_reg->rg[rs2];
    break;
  }
}

// Mash together type_R
//0110011
void helper_extension (uint32_t word, RiscvRegister_t* rscv_reg)
{
  uint32_t rs1;
  uint32_t rs2;
  uint32_t rd;

  rs1 = get_rs1(word);
  rs2 = get_rs2(word);
  rd = get_rd(word);
  uint32_t funct3 = get_funct3(word);

  switch (funct3)
  {
    case 0:
      printf("MUL\n");
      // Multiply two signed numbers
      rscv_reg-> rg[rd] = (int32_t)rscv_reg->rg[rs1] * (int32_t)rscv_reg->rg[rs2];
      break;
    case 1:
      printf("MULH\n");
      rscv_reg-> rg[rd] = (int32_t) rscv_reg->rg[rs1] * (int32_t)rscv_reg->rg[rs2];
      break;
    case 2:
      // One signed with one unsigned
      printf("MULHSU\n");
      rscv_reg-> rg[rd] = (int32_t) rscv_reg->rg[rs1] * rscv_reg->rg[rs2];
      break;
    case 3:
      // Multiply two unsigned numbers
      printf("MULHU\n");
      rscv_reg-> rg[rd] = rscv_reg->rg[rs1] * rscv_reg->rg[rs2];
      break;
    case 4:
      // Divide two signed integers
      printf("DIV\n");
      rscv_reg-> rg[rd] = (int32_t) rscv_reg->rg[rs1] / (int32_t)rscv_reg->rg[rs2];
      break;
    case 5:
      // Divide two unsigned integers
      printf("DIVU\n");
      rscv_reg-> rg[rd] = rscv_reg->rg[rs1] / rscv_reg->rg[rs2];
      break;
    case 6:
      // Modulo of two signed integers
      printf("REM\n");
      rscv_reg-> rg[rd] = (int32_t) rscv_reg->rg[rs1] % (int32_t)rscv_reg->rg[rs2];
      break;
    case 7:
      // Modulo of two unsigned integers
      printf("REMU\n");
      rscv_reg-> rg[rd] = rscv_reg->rg[rs1] % rscv_reg->rg[rs2];
      break;
  }
}


void ex_or_nah (uint32_t word, RiscvRegister_t* rscv_reg)
{
  uint32_t funct7 = get_funct7(word);

  switch (funct7)
  {
    case 0:
      type_R(word, rscv_reg);
      break;
    case 32:
      type_R(word, rscv_reg);
      break;
    case 1:
      helper_extension(word, rscv_reg);
      break;
  }
}

long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file) {

  // Struct for risc-v register and instructions
  struct RiscvRegister rscv_reg;
  rscv_reg.PC = start_addr;

  uint64_t inst_cnt = 0; // Instruction counter
  uint32_t word; // Instruction to be decoded
  
  // Insttruction set
  uint32_t opcode;
  uint32_t rd;
  uint32_t funct3;
  int imm_i;
  int imm_j;
  uint32_t rs1;
  // uint32_t rs2;


  while (inst_cnt <= 50) {

    // Read word (instruction set)
    word = memory_rd_w(mem, rscv_reg.PC);

    // Decode instruction set
    opcode = get_opcode(word);
    rd = get_rd(word);
    funct3 = get_funct3(word);
    rs1 = get_rs1(word);
    // rs2 = get_rs2(word);
    imm_j = get_imm_J(word);
    imm_i = get_imm_I(word);
    
    // Increase instruction count
    inst_cnt++;

    // Pattern matching
    switch(opcode) {
      case 55:
        printf("LUI\n");
        printf("Decimal: %i\n", imm_j);
        rscv_reg.rg[rd] = imm_j;
        break;
      case 23:
        printf("AUIPC\n");
        rscv_reg.rg[rd] = rscv_reg.PC + imm_j;
        break;
      case 111:
        printf("JAL\n");
        rscv_reg.rg[rd] = rscv_reg.PC + 4; // pc + 4
        rscv_reg.PC = rscv_reg.PC + imm_j;
        break;
      case 103:
        printf("JALR\n");
        rscv_reg.rg[rd] = rscv_reg.PC + 4; // pc + 4
        rscv_reg.PC = rscv_reg.rg[rs1] + imm_i;
        break;
      case 99:  
        type_B(word, &rscv_reg);
        break;
      case 51:
        // helper_extension(word); // helper_type_R
        // To differentiate between ex or type_R
        ex_or_nah(word, &rscv_reg);
        break;
      case 3:
        type_I(word, &rscv_reg);
        break;
      case 35:
        type_S(word, &rscv_reg);
        break;
      //type_S2
      case 19:
        type_I2(word, &rscv_reg);
        break;
      // If li	a7,3. LI = ADDI
      // then program is done and out c code should terminal
      // it is an ecall
      case 115:
        printf("ecall\n");
        if (rscv_reg.rg[17] == TERMINATE_3 || rscv_reg.rg[17] == TERMINATE_93) {
          // Terminate program
          return inst_cnt;
        }
        ecall(&rscv_reg);
        break;
    }

    // Debug
    printf("word: %u\n", word);
    printf("opcode: %u\n", opcode);
    printf("funct3: %u\n", funct3);
    printf("pc: %0llx\n", rscv_reg.PC);
    printf("rd: %u\n", rd);
    printf("rs1: %u\n", rs1);
    printf("imm11: %i\n", imm_i);
    printf("imm20: %i\n", imm_j);
    printf("\n");
  }
  // finder_function(opcode);
  return inst_cnt;
}
