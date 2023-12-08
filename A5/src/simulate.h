#ifndef __SIMULATE_H__
#define __SIMULATE_H__

#include "memory.h"
#include "assembly.h"
#include <stdio.h>

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

// RISC-V 32 Register
typedef struct RiscvRegister {
  int rg[32];
} RiscvRegister_t;

// Simuler RISC-V program i givet lager og fra given start adresse
long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file);

#endif
