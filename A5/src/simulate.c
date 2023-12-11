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
  int32_t imm;

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
        printf("IMM=%u\n", imm);
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
  int32_t imm;

  rs1 = get_rs1(word);
  // rs2 = get_rs2(word);
  rd = get_rd(word);
  imm = get_imm_I(word);
  // struct memory *mem = memory_create();

  uint32_t address = rscv_reg->rg[rs1] + imm;
  uint32_t funct3 = get_funct3(word);

  switch (funct3)
  {
    case 0:
      // Load Byte
      printf("LB\n");
      int8_t byte = memory_rd_b(rscv_reg->mem, address);
      rscv_reg->rg[rd] = (int32_t)byte;
      rscv_reg->PC += 4;
      break;
    case 1:
      printf("LH\n");
      int16_t halfword = memory_rd_b(rscv_reg->mem, address);
      rscv_reg->rg[rd] = (int32_t)halfword;
      rscv_reg->PC += 4;
      break;
    case 2:
      printf("LW\n");
      int32_t full_word = memory_rd_b(rscv_reg->mem, address);
      rscv_reg->rg[rd] = (int32_t)full_word;
      rscv_reg->PC += 4;
      break;
    case 4:
      printf("LBU\n");
      uint8_t u_byte = memory_rd_b(rscv_reg->mem, address);
      rscv_reg->rg[rd] = (uint32_t)u_byte;
      rscv_reg->PC += 4;
      break;
    case 5:
      printf("LHU\n");
      uint16_t u_halfword = memory_rd_b(rscv_reg->mem, address);
      rscv_reg->rg[rd] = (uint32_t)u_halfword;
      rscv_reg->PC += 4;
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
  int32_t imm;
  
  
  
  rs1 = get_rs1(word);
  rs2 = get_rs2(word);
  // rd = get_rd(word);
  imm = get_imm_S(word);

  // struct memory *mem = memory_create();
  int32_t address = rscv_reg->rg[rs1] + imm;

  funct3 = get_funct3(word);

  switch (funct3)
  {
    case 0:
      // Store byte in memory
      printf("SB\n");
      int8_t byte = rscv_reg->rg[rs2];
      printf("SB Adress == %u\n",address );
      memory_wr_b(rscv_reg->mem, address, byte);
      rscv_reg->PC += 4;
      break;
    case 1:
      // store halfword in memory 
      printf("SH\n");
      int16_t halfword = rscv_reg->rg[rs2];
      printf("SH Adress == %u\n",address );
      memory_wr_h(rscv_reg->mem, address, halfword);
      rscv_reg->PC += 4;
      break;
    case 2:
      // store word in memory 
      printf("SW\n");
      int32_t full_word = rscv_reg->rg[rs2];
      printf("SW Adress == %u, IMM== %i, RS1==%i, RS2==%i \n",address, imm, rs1, rs2 );
      memory_wr_w(rscv_reg->mem, address, full_word);
      rscv_reg->PC += 4;
      break;
  }
}

//0010011
void type_I2 (uint32_t word, RiscvRegister_t* rscv_reg)
{
  uint32_t funct3;
  uint32_t rs1;
  uint32_t rd;
  int32_t imm;
  uint32_t funct7;

  funct7 = get_funct7(word);
  funct3 = get_funct3(word);
  rd = get_rd(word);
  rs1 = get_rs1(word);
  imm = get_imm_I(word);

  // Debug
  printf("rs1: %u\n", rs1);
  printf("imm_I: %i\n", imm);
  printf("rd: %u\n", rd);

  switch (funct3)
  {
    case 0:
      printf("ADDI\n"); // I-type
      rscv_reg->rg[rd] = rs1 + get_imm_I(word);
      rscv_reg->PC += 4;
      break;
    case 2:
      // Set less than immediate signed
      printf("SLTI\n");
      if (rscv_reg->rg[rs1] < imm)
      {
        rscv_reg->rg[rd] = 1;
        rscv_reg->PC += 4;
      }
      else
      {
        rscv_reg->rg[rd] = 0;
        rscv_reg->PC += 4;
      }
      break;
    case 3:
      // Set less than immediate unsigned
      printf("SLTIU\n");
      if (rscv_reg->rg[rs1] < imm)
      {
        rscv_reg->rg[rd] = 1;
        rscv_reg->PC += 4;
      }
      else
      {
        rscv_reg->rg[rd] = 0;
        rscv_reg->PC += 4;
      }
      break;
    case 4:
      // Exclusive or immediate
      printf("XORI\n");
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] ^ imm;
      rscv_reg->PC += 4;
      break;
    case 6:
      printf("ORI\n"); // I-type
      rscv_reg->rg[rd] = get_imm_I(word) | rs1;
      rscv_reg->PC += 4;
      break;
    case 7:
      printf("ANDI\n");
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] & imm;
      rscv_reg->PC += 4;
      break;
    
    // Next ones are different types with the shamt
    
    case 1:
      // Shift left immidiate
      printf("SLLI\n");
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] << imm;
      rscv_reg->PC += 4;
      break;
    case 5:
      printf("SRLI/SRAI\n");
      if (funct7 == 0){
        printf("SRLI\n");
        rscv_reg->rg[rd] = rscv_reg->rg[rs1] >> imm;
        rscv_reg->PC += 4;
      }
      else
      {
        printf("SRAI\n");
        rscv_reg->rg[rd] = (int32_t)rscv_reg->rg[rs1] >> imm;
        rscv_reg->PC += 4;
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
      printf("ADD/SUB\n");
      if (funct7 == 0)
      {
        rscv_reg->rg[rd] = rscv_reg->rg[rs1] + rscv_reg->rg[rs2];
        rscv_reg->PC += 4;
      }
      else
      {
        rscv_reg->rg[rd] = rscv_reg->rg[rs1] - rscv_reg->rg[rs2];
        rscv_reg->PC += 4;
      }
      printf("ADD rg[rd]: %lli\n", rscv_reg->rg[rd]);
      break;
    case 1:
      printf("SLL\n");
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] << rscv_reg->rg[rs2];
        rscv_reg->PC += 4;
      printf("rg[rd]: %lli\n", rscv_reg->rg[rd]);
      break;
    case 2:
      // set less than if rs1 < rs2 else 0
      printf("SLT\n");
      if ((int32_t) rscv_reg->rg[rs1] < (int32_t)rscv_reg->rg[rs2]) {
        rscv_reg->rg[rd] = 1;
        rscv_reg->PC += 4;

      } else {
        rscv_reg->rg[rd] = 0;
        rscv_reg->PC += 4;

      }
      break;
    case 3:
      // Set less unsigned cast to unsigned
      printf("SLTU\n");
      if (rscv_reg->rg[rs1] < rscv_reg->rg[rs2]) {
        rscv_reg->rg[rd] = 1;
        rscv_reg->PC += 4;

      } else {
        rscv_reg->rg[rd] = 0;
        rscv_reg->PC += 4;

      }
      printf("rg[rd]: %lli\n", rscv_reg->rg[rd]);
      break;
    case 4:
      // Exclusive or
      printf("XOR\n");
      rscv_reg->rg[rd] = rscv_reg->rg[rs1]^rscv_reg->rg[rs2];
      rscv_reg->PC += 4;

      break;
    case 5:
      printf("SRL/SRA\n");
      if (funct7 == 0)
      {
        rscv_reg->rg[rd] = rscv_reg->rg[rs1] >> rscv_reg->rg[rs2];
        rscv_reg->PC += 4;

      }
      else
      {
        rscv_reg->rg[rd] = (int32_t)rscv_reg->rg[rs1] >> rscv_reg->rg[rs2];
        rscv_reg->PC += 4;
      }
      break;
    case 6:
      printf("OR\n");
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] | rscv_reg->rg[rs2];
      rscv_reg->PC += 4;

      break;
    case 7:
      printf("AND\n");
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] & rscv_reg->rg[rs2];
      rscv_reg->PC += 4;

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
      rscv_reg->PC += 4;
      break;
    case 1:
      printf("MULH\n");
      rscv_reg-> rg[rd] = (int32_t) rscv_reg->rg[rs1] * (int32_t)rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
    case 2:
      // One signed with one unsigned
      printf("MULHSU\n");
      rscv_reg-> rg[rd] = (int32_t) rscv_reg->rg[rs1] * rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
    case 3:
      // Multiply two unsigned numbers
      printf("MULHU\n");
      rscv_reg-> rg[rd] = rscv_reg->rg[rs1] * rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
    case 4:
      // Divide two signed integers
      printf("DIV\n");
      rscv_reg-> rg[rd] = (int32_t) rscv_reg->rg[rs1] / (int32_t)rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
    case 5:
      // Divide two unsigned integers
      printf("DIVU\n");
      rscv_reg-> rg[rd] = rscv_reg->rg[rs1] / rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
    case 6:
      // Modulo of two signed integers
      printf("REM\n");
      rscv_reg-> rg[rd] = (int32_t) rscv_reg->rg[rs1] % (int32_t)rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
    case 7:
      // Modulo of two unsigned integers
      printf("REMU\n");
      rscv_reg-> rg[rd] = rscv_reg->rg[rs1] % rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
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

  rscv_reg.mem = memory_create();

  uint64_t inst_cnt = 0; // Instruction counter
  uint32_t word; // Instruction to be decoded
  
  // Insttruction set
  uint32_t opcode;
  uint32_t rd;
  uint32_t rs1;

  while (inst_cnt <= 500) {

    // Read word (instruction set)
    word = memory_rd_w(mem, rscv_reg.PC);

    // Decode instruction set
    opcode = get_opcode(word);
    rs1 = get_rs1(word);
    rd = get_rd(word);
    
    // Increase instruction count
    inst_cnt++;

    // Debug
    printf("word: %u\n", word);
    printf("opcode: %u\n", opcode);
    printf("pc: %0llx\n", rscv_reg.PC);
    printf("rd: %u\n", rd);
    printf("rs1: %u\n", rs1);

    // Pattern matching
    switch(opcode) {
      case 55:
        rscv_reg.rg[rd] = get_imm_U(word);
        rscv_reg.PC += 4;
        printf("LUI\n"); // U-type
        break;
      case 23:
        rscv_reg.rg[rd] = rscv_reg.PC + get_imm_U(word);
        rscv_reg.PC += 4;
        printf("imm-u: %i\n", get_imm_U(word));
        printf("AUIPC\n"); // U-type
        break;
      case 111:
        rscv_reg.rg[rd] = rscv_reg.PC + 4;
        rscv_reg.PC = rscv_reg.PC + get_imm_J(word);
        printf("JAL\n"); // J-type
        goto jump;
      case 103:
        rscv_reg.rg[rd] = rscv_reg.PC + 4;
        rscv_reg.PC = rscv_reg.rg[rs1] + get_imm_I(word);
        printf("rg[1]: %0llx\n", rscv_reg.rg[1]);
        printf("JALR\n"); // I-type
        goto jump;
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

    // Increment PC
    // rscv_reg.PC += 4;

    // JAL and JALR;
    jump:

    printf("\n");
  }
  // finder_function(opcode);
  return inst_cnt;
}
