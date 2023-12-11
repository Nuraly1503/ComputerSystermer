#ifndef __SIMULATE_H__
#define __SIMULATE_H__

#include "memory.h"
#include "assembly.h"
#include "decode_helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Register
#define REGISTER_LEN  32
#define a0            10
#define a7            17

// Ecall
#define TERMINATE_3   3 
#define TERMINATE_93  93

// RISC-V Register
typedef struct RiscvRegister {
  int64_t rg[REGISTER_LEN];  // 32 risc-v registers (x0-x31)
  uint64_t PC;  // Program counter: address of the next instruction
  struct memory *mem;
}RiscvRegister_t

// Simuler RISC-V program i givet lager og fra given start adresse
long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file);

#endif
