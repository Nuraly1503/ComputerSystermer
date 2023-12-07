#include "memory.h"
#include "assembly.h"
#include <stdio.h>

#define REGISTER_LEN  32
#define INSTR_LEN     32
#define OPCODE_LEN    7
#define RD_LEN        5
#define FUNCT3_LEN    3
#define RS1_LEN       4
#define RS2_LEN       5
#define FUNCT7_LEN    7


unsigned get_opcode(unsigned word) {
  unsigned opcode;
  opcode = word << (INSTR_LEN - OPCODE_LEN);
  opcode = opcode >> (INSTR_LEN - OPCODE_LEN);
  return opcode;
}

unsigned get_rd(unsigned word) {
  unsigned rd;
  rd = word >> OPCODE_LEN;
  rd = rd & 0x1F;
  return rd;
}

unsigned get_funct3(unsigned word) {
  unsigned funct3;
  funct3 = word >> (OPCODE_LEN + RD_LEN);
  funct3 = funct3 & 0x7;
  return funct3;
}

unsigned get_rs1(unsigned word) {
  unsigned rs1;
  rs1 = word >> (OPCODE_LEN + RD_LEN + FUNCT3_LEN);
  rs1 = rs1 & 0xF;
  return rs1;
}

unsigned get_rs2(unsigned word) {
  unsigned rs2;
  rs2 = word >> (OPCODE_LEN + RD_LEN + FUNCT3_LEN + RS1_LEN);
  rs2 = rs2 & 0x1F;
  return rs2;
}

unsigned get_funct7(unsigned word) {
  unsigned funct7;
  funct7 = word >> (INSTR_LEN - FUNCT7_LEN);
  return funct7;
}

long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file) {

  int reg[REGISTER_SIZE];
  long int pc = start_addr;  // Program counter: address of the next instruction
  long int inst_cnt = 0; // Instruction counter
  unsigned word;
  unsigned opcode;
  unsigned funct3;

  while (inst_cnt <= 10) {

    // Read word (instruction set)
    word = memory_rd_w(mem, pc);

    // Decode instruction set
    opcode = get_opcode(word);
    funct3 = get_funct3(word);


    // Program counter
    pc += 4;

    printf("word: %i\n", word);
    printf("opcode: %u\n", opcode);
    printf("funct3: %u\n", funct3);
    printf("pc: %0lx\n", pc);
   
    // Increase instruction count
    inst_cnt++;

    // Pattern matching
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


