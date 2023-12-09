#include "memory.h"
#include "assembly.h"
#include "simulate.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Instruction set
#define INSTR_LEN     32
#define OPCODE_LEN    7
#define RD_LEN        5
#define FUNCT3_LEN    3
#define RS1_LEN       5
#define RS2_LEN       5
#define FUNCT7_LEN    7

// Instruction Set helper functions
uint32_t get_opcode(uint32_t word);
uint32_t get_rd(uint32_t word);
uint32_t get_funct3(uint32_t word);
uint32_t get_rs1(uint32_t word);
uint32_t get_rs2(uint32_t word);
uint32_t get_funct7(uint32_t word);

// Immidiates helper functions
int32_t get_imm_I(uint32_t word);
int32_t get_imm_S(uint32_t word);
int32_t get_imm_B(uint32_t word);
int32_t get_imm_U(uint32_t word);
int32_t get_imm_J(uint32_t word);

