#include "./memory.h"
#include "./assembly.h"
#include "./simulate.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Register
#define REGISTER_LEN  32
#define a0            10
#define a7            17

// Instruction set
#define INSTR_LEN     32
#define OPCODE_LEN    7
#define RD_LEN        5
#define FUNCT3_LEN    3
#define RS1_LEN       5
#define RS2_LEN       5
#define FUNCT7_LEN    7

// Ecall
#define TERMINATE_3   3 
#define TERMINATE_93  93


// Instruction Set helper functions
unsigned get_opcode(unsigned word) {
  unsigned opcode;
  opcode = word << (INSTR_LEN - OPCODE_LEN);
  opcode = opcode >> (INSTR_LEN - OPCODE_LEN);
  return opcode;
}

unsigned get_rd(unsigned word) {
  unsigned rd;
  rd = word >> OPCODE_LEN;
  rd = rd & 0x1F;
  return rd;
}

unsigned get_funct3(unsigned word) {
  unsigned funct3;
  funct3 = word >> (OPCODE_LEN + RD_LEN);
  funct3 = funct3 & 0x7;
  return funct3;
}

unsigned get_rs1(unsigned word) {
  unsigned rs1;
  rs1 = word >> (OPCODE_LEN + RD_LEN + FUNCT3_LEN);
  rs1 = rs1 & 0xF;
  return rs1;
}

unsigned get_rs2(unsigned word) {
  unsigned rs2;
  rs2 = word >> (OPCODE_LEN + RD_LEN + FUNCT3_LEN + RS1_LEN);
  rs2 = rs2 & 0x1F;
  return rs2;
}

unsigned get_funct7(unsigned word) {
  unsigned funct7;
  funct7 = word >> (INSTR_LEN - FUNCT7_LEN);
  return funct7;
}

int get_imm11(unsigned word) {
  int imm11;
  imm11 = word >> (RS1_LEN + FUNCT3_LEN + RD_LEN + OPCODE_LEN);
  return imm11;
}

// Ecall helper function
void ecall(RiscvRegister_t* inst_reg) {
  unsigned var = inst_reg->rg[a7];
  switch(var) {
    case 1:
      // returner "getchar()" i A0
      printf("getchar()\n");
      inst_reg->rg[a0] = getchar();
      break;
    case 2:
      // udfør "putchar(c)", hvor c tages fra A0
      printf("putchar()\n");
      putchar(inst_reg->rg[a0]);
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
void type_B (unsigned word ) 
{
  unsigned func = get_funct3(word);

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
void type_I (unsigned word ) 
{
  unsigned func = get_funct3(word);

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
void type_S (unsigned word ) 
{
  unsigned func = get_funct3(word);

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
void type_S2 (unsigned word, RiscvRegister_t* inst_reg)
{
  unsigned func;
  unsigned rs1;
  unsigned rd;
  unsigned imm11;

  func = get_funct3(word);

  switch (func)
  {
  case 0:
    printf("ADDI\n");
      rs1 = get_rs1(word);
      imm11 = get_imm11(word);
      rd = get_rd(word);
      printf("rs1: %u\n", rs1);
      printf("imm11: %i\n", imm11);
      printf("rd: %u\n", rd);
      inst_reg->rg[rd] = imm11 + rs1;
      printf("reg[rs1]: %i\n", inst_reg->rg[rd]);
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
    rs1 = get_rs1(word);
    imm11 = get_imm11(word);
    rd = get_rd(word);
    printf("rs1: %u\n", rs1);
    printf("imm11: %i\n", imm11);
    printf("rd: %u\n", rd);
    int tmp = imm11 | rs1;
    printf("tmp: %i\n", tmp);
    inst_reg->rg[rd] = tmp;
    printf("rg[rd]: %i\n", inst_reg->rg[rd]);
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
void type_R (unsigned word ) 
{
  unsigned func = get_funct3(word);

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
void helper_extension (unsigned word)
{
  unsigned func = get_funct3(word);

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


void ex_or_nah (unsigned word)
{
  unsigned funct7 = get_funct7(word);

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

  struct RiscvRegister inst_reg;
  long int pc = start_addr;  // Program counter: address of the next instruction
  long int inst_cnt = 0; // Instruction counter
  unsigned long word;
  unsigned opcode;
  unsigned funct3;

  while (inst_cnt <= 5) {

    // Read word (instruction set)
    word = memory_rd_w(mem, pc);

    // Decode instruction set
    opcode = get_opcode(word);
    funct3 = get_funct3(word);

    // Program counter
    pc += 4;

    printf("word: %lu\n", word);
    printf("opcode: %u\n", opcode);
    printf("funct3: %u\n", funct3);
    printf("pc: %0lx\n", pc - 4);
   
    // Increase instruction count
    inst_cnt++;

    // Pattern matching
    switch(opcode) {
      case 55:
        printf("LUI\n");
        break;
      case 23:
        printf("AUIPC\n");
        break;
      case 111:
        printf("JAL\n");
        break;
      case 103:
        printf("JALR\n");
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
        type_S2(word, &inst_reg);
        break;
      // If li	a7,3. LI = ADDI
      // then program is done and out c code should terminal
      // it is an ecall
      case 115:
        printf("ecall\n");
        if (inst_reg.rg[17] == TERMINATE_3 || inst_reg.rg[17] == TERMINATE_93) {
          // Terminate program
          return inst_cnt;
        }
        ecall(&inst_reg);
        break;
    }
  }
  // finder_function(opcode);
  return inst_cnt;
}
