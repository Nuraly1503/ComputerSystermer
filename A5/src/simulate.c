#include "memory.h"
#include "assembly.h"
#include <stdio.h>

#define REGISTER_SIZE 32

long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file) {

  int reg[REGISTER_SIZE];
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
        // Hjælpefunction til type B
        break;
      case 3:
        // Hjælpefunctio type I
        break;
      case 35:
        //hjælpefunction til type S
        break;
      case 19:
        //Hjækpefunction til type I  plus shamtforskellig fra 3
        break; 
      case 51:
        // Hjælpe function til ? plus standard extensions.
        break;
      // case 19:
      //   printf("MV/ADDI\n");
      //   break;

      // If li	a7,3. LI = ADDI
      // then program is done and out c code should terminal
      // it is an ecall
      case 115: //1110011
        return inst_cnt;
        break;
      default:
        break;
    }
  }
  // finder_function(opcode);
  return inst_cnt;
}

void type_B (unsigned word ) 
{

  switch(1)// Func3 {
  {
    case 
  }

}


