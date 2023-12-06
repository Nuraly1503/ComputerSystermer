#include "memory.h"
#include "assembly.h"
#include <stdio.h>

#define WORD_SIZE  32

long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file) {

  long int pc = start_addr;  // Program counter: address of the next instruction
  long int inst_cnt = 0; // Instruction counter
  unsigned word;
  unsigned tmp;
  unsigned opcode;

  while (inst_cnt <= 5) {

    // Read word (instruction set)
    word = memory_rd_w(mem, pc);

    // Word to byte array
    // use bitwise masking
    tmp = (word << 25);
    opcode = (tmp >> 25);
    pc += 4;

    printf("word: %i\n", word);
    printf("tmp: %i\n", tmp);
    printf("opcode: %u\n", opcode);
    printf("pc: %0lx\n", pc);
   
    // Increase instruction count
    inst_cnt++;

    // Pattern matching
    // switch statement
    switch(opcode) {
      case 19:
        printf("MV/ADDI\n");
        break;
      case 23:
        printf("AUIPC\n");
        break;
      case 55:
        printf("LUI\n");
        break;
      case 103:
        printf("JALR\n");
        break;

      // If li	a7,3. LI = ADDI
      // then program is done and out c code should terminal
      // it is an ecall
      case 115: //1110011
        return inst_cnt;
    }
  }

  return inst_cnt;
}
