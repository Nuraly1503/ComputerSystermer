#include "memory.h"
#include "assembly.h"
#include <stdio.h>

#define WORD_SIZE  32

long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file) {

  long int pc = start_addr;  // Program counter: address of the next instruction
  long int inst_cnt = 0; // Instruction counter
  int word;
  int tmp;
  int opcode;

  while (inst_cnt <= 10) {

    word = memory_rd_w(mem, pc);

    // Word to byte array
    // use bitwise masking
    tmp = (word << 25);
    opcode = (tmp >> 25);

    printf("%i\n", word);
    printf("%i\n", tmp);
    printf("%i\n", opcode);
   
    // Increase instruction count
    inst_cnt++;

    // Pattern matching
    // switch statement
    switch(opcode) {
      case 55: //LUI
        printf("LUI\n");
        return inst_cnt;
      // If li	a7,3. LI = ADDI
      // then program is done and out c code should terminal
      // it is an ecall
      case 115: //1110011
        return inst_cnt;
      default:
        return;
    }
  }

}
