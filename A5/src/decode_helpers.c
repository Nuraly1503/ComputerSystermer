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
// Imm are signed 32bit integers
// Reference: https://fmash16.github.io/content/posts/riscv-emulator-in-c.html
int32_t get_imm_I(uint32_t word) {
  // imm[11:0] = inst[31:20]
  int32_t imm_i;
  imm_i = ((int32_t) word) >> (RS1_LEN + FUNCT3_LEN + RD_LEN + OPCODE_LEN);
  return imm_i;
}

int32_t get_imm_S(uint32_t word) {
  // imm[11:5|4:0] = inst[31:25|11:7]
  int32_t imm_s;
  imm_s = ((int32_t) (word & 0xfe000000)) >> (INSTR_LEN - OPCODE_LEN - RD_LEN)
    | ((word >> OPCODE_LEN) & 0x1f);
  return imm_s;
}

int32_t get_imm_B(uint32_t word) {
  // imm[12|10:5|4:1|11] = inst[31|30:25|11:8|7]
  int32_t imm_b;
  imm_b = ((int32_t) (word & 0x80000000) >> 19) //imm[12] = inst[31]
    | ((word & 0x7e000000) >> 20) //imm[10:5] = inst[30:25]
    | ((word & 0x00000f00) >> 7) //imm[4:1] = inst[11:8]
    | ((word & 0x00000080) << 4); //imm[11] = inst[7]
  return imm_b;
}

int32_t get_imm_U(uint32_t word) {
  // imm[31:12] = inst[31:12]
  // shifted left by 12 bits   
  int32_t imm_u;
  imm_u = ((int32_t) word) >> (OPCODE_LEN + RD_LEN);
  imm_u = imm_u << (OPCODE_LEN + RD_LEN);
  return imm_u;
}

int32_t get_imm_J(uint32_t word) {
  // imm[20|10:1|11|19:12] = inst[31|30:21|20|19:12]
  int32_t imm_j;
  imm_j = ((int32_t) (word & 0x80000000) >> 11) // imm[20] = inst[31]
    | ((word & 0x7fe00000) >> 20) // imm[10:1] = inst[30:21]
    | ((word & 0x00100000) >> 9) // imm[11] = inst[20]
    | (word & 0x000ff000); // imm[19:12] = inst[19:12]
  return imm_j;
}

