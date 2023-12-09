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
void type_B (uint32_t word ) 
{
  uint32_t func = get_funct3(word);

  switch(func)
  {
    case 0:
      printf("BEQ\n");
      break;
    case 1:
      printf("BNE\n");
      break;
    case 4:
      printf("BLT\n");
      break;
    case 5:
      printf("BGE\n");
      break;
    case 6:
      printf("BLTU\n");
      break;
    case 7:
      printf("BGEU\n");
      break;
  };
}

// 0000011
void type_I (uint32_t word ) 
{
  uint32_t func = get_funct3(word);

  switch (func)
  {
  case 0:
    printf("LB\n");
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
void type_S (uint32_t word ) 
{
  uint32_t func = get_funct3(word);

  switch (func)
  {
  case 0:
    printf("SB\n");
    break;
  case 1:
    printf("SH\n");
    break;
  case 2:
    printf("SW\n");
    break;
  }
}

//0010011
void type_S2 (uint32_t word, RiscvRegister_t* rscv_reg)
{
  uint32_t funct3;
  uint32_t rs1;
  uint32_t rd;
  uint32_t imm11;

  funct3 = get_funct3(word);
  rd = get_rd(word);
  rs1 = get_rs1(word);
  imm11 = get_imm11(word);

  // Debug
  printf("rs1: %u\n", rs1);
  printf("imm11: %i\n", imm11);
  printf("rd: %u\n", rd);

  switch (funct3)
  {
  case 0:
    printf("ADDI\n");
      rscv_reg->rg[rd] = imm11 + rs1;
      printf("reg[rs1]: %lli\n", rscv_reg->rg[rd]);
    break;
  case 2:
    printf("SLTI\n");
    break;
  case 3:
    printf("SLTIU\n");
    break;
  case 4:
    printf("XORI\n");
    break;
  case 6:
    printf("ORI\n");
    rscv_reg->rg[rd] = imm11 | rs1;
    printf("rg[rd]: %lli\n", rscv_reg->rg[rd]);
    break;
  case 7:
    printf("ANDI\n");
    break;
  
  // Next ones are different types with the shmat
  
  case 1:
    printf("SLLI\n");
    break;
  case 5:
    printf("SRLI/SRAI\n");            //OBS
    break;
  }
}

//0110011
void type_R (uint32_t word ) 
{
  uint32_t func = get_funct3(word);

  switch (func)
  {
  case 0:
    printf("ADD/SUB\n");               /// OBS 
    break;
  case 1:
    printf("SLL\n");
    break;
  case 2:
    printf("SLT\n");
    break;
  case 3:
    printf("SLTU\n");
    break;
  case 4:
    printf("XOR\n");
    break;
  case 5:
    printf("SRL/SRA\n");             /// OBS
    break;
  case 6:
    printf("OR\n");
    break;
  case 7:
    printf("AND\n");
    break;
  }
}

// Mash together type_R
//0110011
void helper_extension (uint32_t word)
{
  uint32_t func = get_funct3(word);

  switch (func)
  {
    case 0:
      printf("MUL\n");
      break;
    case 1:
      printf("MULH\n");
      break;
    case 2:
      printf("MULHSU\n");
      break;
    case 3:
      printf("MULHU\n");
      break;
    case 4:
      printf("DIV\n");
      break;
    case 5:
      printf("DIVU\n");
      break;
    case 6:
      printf("REM\n");
      break;
    case 7:
      printf("REMU\n");
      break;
  }
}


void ex_or_nah (uint32_t word)
{
  uint32_t funct7 = get_funct7(word);

  switch (funct7)
  {
    case 0:
      //helper_type_R(word);
      break;
    case 32:
      //helper_type_R(word);
      break;
    case 1:
      //helper_extension(word);
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
  int imm11;
  int imm20;
  uint32_t rs1;
  uint32_t rs2;


  while (inst_cnt <= 50) {

    // Read word (instruction set)
    word = memory_rd_w(mem, rscv_reg.PC);

    // Decode instruction set
    opcode = get_opcode(word);
    rd = get_rd(word);
    funct3 = get_funct3(word);
    rs1 = get_rs1(word);
    rs2 = get_rs2(word);
    imm20 = get_imm20(word);
    imm11 = get_imm11(word);
    
    // Increase instruction count
    inst_cnt++;

    // Pattern matching
    switch(opcode) {
      case 55:
        printf("LUI\n");
        printf("Decimal: %i\n", imm20);
        rscv_reg.rg[rd] = imm20;
        break;
      case 23:
        printf("AUIPC\n");
        rscv_reg.rg[rd] = rscv_reg.PC + imm20;
        break;
      case 111:
        printf("JAL\n");
        rscv_reg.rg[rd] = rscv_reg.PC + 4; // pc + 4
        rscv_reg.PC = rscv_reg.PC + imm20;
        break;
      case 103:
        printf("JALR\n");
        rscv_reg.rg[rd] = rscv_reg.PC + 4; // pc + 4
        rscv_reg.PC = rscv_reg.rg[rs1] + imm11;
        break;
      case 99:  
        type_B(word);
        break;
      case 51:
        // helper_extension(word); // helper_type_R
        // To differentiate between ex or type_R
        ex_or_nah(word);
        break;
      case 3:
        type_I(word);
        break;
      case 35:
        type_S(word);
        break;
      //type_S2
      case 19:
        type_S2(word, &rscv_reg);
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
    printf("imm11: %i\n", imm11);
    printf("imm20: %i\n", imm20);
    printf("\n");
  }
  // finder_function(opcode);
  return inst_cnt;
}
