#include "memory.h"
#include "assembly.h"
#include "simulate.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decode_helpers.h"

// Instruction Set helper functions
uint32_t get_opcode(uint32_t word) {
  // inst[6:0]
  uint32_t opcode;
  opcode = word << (INSTR_LEN - OPCODE_LEN);
  opcode = opcode >> (INSTR_LEN - OPCODE_LEN);
  return opcode;
}

uint32_t get_rd(uint32_t word) {
  // inst[11:7]
  uint32_t rd;
  rd = word >> OPCODE_LEN;
  rd = rd & 0x1F;
  return rd;
}

uint32_t get_funct3(uint32_t word) {
  // inst[14:12]
  uint32_t funct3;
  funct3 = word >> (OPCODE_LEN + RD_LEN);
  funct3 = funct3 & 0x7;
  return funct3;
}

uint32_t get_rs1(uint32_t word) {
  // inst[19:15]
  uint32_t rs1;
  rs1 = word >> (OPCODE_LEN + RD_LEN + FUNCT3_LEN);
  rs1 = rs1 & 0xF;
  return rs1;
}

uint32_t get_rs2(uint32_t word) {
  // inst[24:20]
  uint32_t rs2;
  rs2 = word >> (OPCODE_LEN + RD_LEN + FUNCT3_LEN + RS1_LEN);
  rs2 = rs2 & 0x1F;
  return rs2;
}

uint32_t get_funct7(uint32_t word) {
  // inst[31:25]
  uint32_t funct7;
  funct7 = word >> (INSTR_LEN - FUNCT7_LEN);
  return funct7;
}



// Immidiates helper functions
int get_imm11(uint32_t word) {
  int imm11;
  imm11 = word >> (RS1_LEN + FUNCT3_LEN + RD_LEN + OPCODE_LEN);
  //imm11 = imm11 << (RS1_LEN + FUNCT3_LEN + RD_LEN + OPCODE_LEN);
  return imm11;
}

int get_imm20(uint32_t word) {
  uint32_t imm20;
  imm20 = word >> (OPCODE_LEN + RD_LEN);
  imm20 = imm20 << (OPCODE_LEN + RD_LEN);
  return imm20;
}