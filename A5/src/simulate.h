#ifndef __SIMULATE_H__
#define __SIMULATE_H__

#include "memory.h"
#include "assembly.h"
#include "decode_helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Opcodes
#define ECALL         115
#define TYPE_B        99
#define TYPE_R        51
#define TYPE_I        3
#define TYPE_I2       19
#define TYPE_S        35

// Register
#define REGISTER_LEN  32
#define a0            10
#define a7            17

// Ecall
#define GETCHAR       1
#define PUTCHAR       2
#define TERMINATE_3   3 
#define TERMINATE_93  93

// RISC-V Register
typedef struct RiscvRegister {
  int32_t rg[REGISTER_LEN];  // 32 risc-v registers (x0-x31)
  int32_t PC;  // Program counter: address of the next instruction
} RiscvRegister_t;

// Output log
typedef struct Log {
  char* instr;
  int32_t addr;
} Log_t;

// Simuler RISC-V program i givet lager og fra given start adresse
long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file);

// Helper functions
void helper_extension (uint32_t word, RiscvRegister_t* rscv_reg, Log_t* log);
void type_R (uint32_t word, RiscvRegister_t* rscv_reg, Log_t* log);
void type_B (uint32_t word, RiscvRegister_t* rscv_reg, Log_t* log);
void type_I (uint32_t word, RiscvRegister_t* rscv_reg, struct memory *mem, Log_t* log);
void type_S (uint32_t word, RiscvRegister_t* rscv_reg, struct memory *mem, Log_t* log);
void type_I2 (uint32_t word, RiscvRegister_t* rscv_reg, Log_t* log);

#endif
