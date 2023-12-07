#ifndef __SIMULATE_H__
#define __SIMULATE_H__

#include "memory.h"
#include "assembly.h"
#include <stdio.h>

// RISC-V 32 Register
typedef struct RiscvRegister {
  int rg[32];
} RiscvRegister_t;

// Simuler RISC-V program i givet lager og fra given start adresse
long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file);

#endif
