#include "memory.h"
#include "assembly.h"
#include "simulate.h"
#include "decode_helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


long int simulate(struct memory *mem, struct assembly *as, int start_addr, FILE *log_file) {

  // Struct for risc-v register and instructions
  struct RiscvRegister rscv_reg;
  memset(rscv_reg.rg, 0, 32);
  rscv_reg.PC = start_addr;

  uint64_t inst_cnt = 0; // Instruction counter
  uint32_t word; // Instruction to be decoded
  
  // Insttruction set
  uint32_t opcode;
  uint32_t funct7;
  uint32_t rd;
  uint32_t rs1;
  int32_t rg_rs1;
  int32_t ecall_val;

  while (1) {
    // Read word (instruction set)
    word = memory_rd_w(mem, rscv_reg.PC);

    // Decode instruction set
    opcode = get_opcode(word);
    rs1 = get_rs1(word);
    rd = get_rd(word);
    funct7 = get_funct7(word);
    rg_rs1 = rscv_reg.rg[rs1];
    ecall_val = rscv_reg.rg[a7];
    rscv_reg.rg[0] = 0; // x0 should always be zero
    
    // Increase instruction count
    inst_cnt++;

    // Debug
    // printf("word: %u\n", word);
    // printf("opcode: %u\n", opcode);
    // printf("pc: %0llx\n", rscv_reg.PC);
    // printf("rd: %u\n", rd);
    // printf("rs1: %u\n", rs1);
    // printf("rs2: %u\n", rs2);
    // printf("R[0]==%i\n", rscv_reg.rg[0]);

    // Pattern matching
    switch(opcode) {
      case 55:
        rscv_reg.rg[rd] = get_imm_U(word);
        rscv_reg.PC += 4;
        // printf("LUI\n"); // U-type
        break;
      case 23:
        rscv_reg.rg[rd] = rscv_reg.PC + get_imm_U(word);
        rscv_reg.PC += 4;
        // printf("imm-u: %i\n", get_imm_U(word));
        // printf("AUIPC\n"); // U-type
        break;
      case 111:
        rscv_reg.rg[rd] = rscv_reg.PC + 4;
        rscv_reg.PC = rscv_reg.PC + get_imm_J(word);
        // printf("imm: %i\n", get_imm_J(word));
        // printf("JAL\n"); // J-type
        break;
      case 103:
        rscv_reg.rg[rd] = rscv_reg.PC + 4;
        rscv_reg.PC = rg_rs1 + get_imm_I(word);
        // printf("imm: %i\n", get_imm_I(word));
        // printf("rg[1]: %0x\n", rscv_reg.rg[1]);
        // printf("JALR\n"); // I-type
        break;
      case TYPE_B:  
        type_B(word, &rscv_reg);
        break;
      case TYPE_R:
        // helper_extension(word); // helper_type_R
        // To differentiate between ex or type_R
        // ex_or_nah(word, &rscv_reg);
        switch (funct7) {
          case 0:
            type_R(word, &rscv_reg);
            break;
          case 32:
            type_R(word, &rscv_reg);
            break;
          case 1:
            helper_extension(word, &rscv_reg);
            break;
        }
        break;
      case TYPE_I:
        type_I(word, &rscv_reg, mem);
        break;
      case TYPE_I2:
        type_I2(word, &rscv_reg);
        break;
      case TYPE_S:
        type_S(word, &rscv_reg, mem);
        break;
      
      case ECALL:
        // printf("ecall\n");
        // printf("R[a7]==%i\n", ecall_val);
        switch(ecall_val) {
          case GETCHAR:
            // returner "getchar()" i A0
            // printf("getchar()\n");
            rscv_reg.rg[a0] = getchar();
            rscv_reg.PC += 4;
            break;
          case PUTCHAR:
            // udfør "putchar(c)", hvor c tages fra A0
            // printf("putchar()\n");
            putchar(rscv_reg.rg[a0]);
            rscv_reg.PC += 4;
            break;
          case TERMINATE_3:
          case TERMINATE_93:
            // afslut simulationen
            return inst_cnt;
          default:
            break;
        }
    }

    // printf("\n");
  }

  return inst_cnt;
}


/* 
 * Immidiate Encoding Variants helper functions 
 */

// Mash together type_R
//0110011
void helper_extension (uint32_t word, RiscvRegister_t* rscv_reg) {
  uint32_t rs1 = get_rs1(word);
  uint32_t rs2 = get_rs2(word);
  uint32_t rd = get_rd(word);
  uint32_t funct3 = get_funct3(word);

  switch (funct3) {
    case 0: // MUL
      // printf("MUL\n");
      // Multiply two signed numbers
      rscv_reg-> rg[rd] = (int32_t)rscv_reg->rg[rs1] * (int32_t)rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
    case 1: // MULH
      // printf("MULH\n");
      rscv_reg-> rg[rd] = ((int64_t) rscv_reg->rg[rs1] * (int64_t) rscv_reg->rg[rs2]) >> 32;
      rscv_reg->PC += 4;
      break;
    case 2: // MULHSU
      // One signed with one unsigned
      // printf("MULHSU\n");
      rscv_reg-> rg[rd] = ((int64_t) rscv_reg->rg[rs1] * (uint64_t) rscv_reg->rg[rs2]) >> 32;
      rscv_reg->PC += 4;
      break;
    case 3: // MULHU
      // Multiply two unsigned numbers
      // printf("MULHU\n");
      rscv_reg-> rg[rd] = ((uint64_t) rscv_reg->rg[rs1] * (uint64_t) rscv_reg->rg[rs2]) >> 32;
      rscv_reg->PC += 4;
      break;
    case 4: // DIV 
      // Divide two signed integers
      // printf("DIV\n");
      rscv_reg-> rg[rd] = (int32_t) rscv_reg->rg[rs1] / (int32_t)rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
    case 5: //DIVU
      // Divide two unsigned integers
      // printf("DIVU\n");
      rscv_reg-> rg[rd] = (uint32_t) rscv_reg->rg[rs1] / (uint32_t) rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
    case 6: // REM
      // Modulo of two signed integers
      // printf("REM\n");
      rscv_reg-> rg[rd] = (int32_t) rscv_reg->rg[rs1] % (int32_t)rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
    case 7: // REMU
      // Modulo of two unsigned integers
      // printf("REMU\n");
      rscv_reg-> rg[rd] = (uint32_t) rscv_reg->rg[rs1] % (uint32_t) rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
  }
}

//0110011
void type_R (uint32_t word, RiscvRegister_t* rscv_reg) {
  uint32_t rs2 = get_rs2(word);
  uint32_t rd = get_rd(word);
  uint32_t rs1 = get_rs1(word);
  uint32_t funct3 = get_funct3(word);
  uint32_t funct7 = get_funct7(word);

  switch (funct3) {
    case 0: // ADD, SUB
      // printf("ADD/SUB\n");
      if (funct7 == 0) {
        rscv_reg->rg[rd] = rscv_reg->rg[rs1] + rscv_reg->rg[rs2];
        rscv_reg->PC += 4;
      } else {
        rscv_reg->rg[rd] = rscv_reg->rg[rs1] - rscv_reg->rg[rs2];
        rscv_reg->PC += 4;
      }
      // printf("ADD rg[rd]: %i\n", rscv_reg->rg[rd]);
      break;
    case 1: // SLL
      // printf("SLL\n");
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] << rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      // printf("rg[rd]: %i\n", rscv_reg->rg[rd]);
      break;
    case 2: // SLT
      // set less than if rs1 < rs2 else 0
      // printf("SLT\n");
      if ((int32_t) rscv_reg->rg[rs1] < (int32_t)rscv_reg->rg[rs2]) {
        rscv_reg->rg[rd] = 1;
        rscv_reg->PC += 4;
      } else {
        rscv_reg->rg[rd] = 0;
        rscv_reg->PC += 4;
      }
      break;
    case 3: // SLTU
      // Set less unsigned cast to unsigned
      // printf("SLTU\n");
      if ((uint32_t) rscv_reg->rg[rs1] < (uint32_t) rscv_reg->rg[rs2]) {
        rscv_reg->rg[rd] = 1;
        rscv_reg->PC += 4;
      } else {
        rscv_reg->rg[rd] = 0;
        rscv_reg->PC += 4;
      }
      // printf("rg[rd]: %i\n", rscv_reg->rg[rd]);
      break;
    case 4: // XOR
      // Exclusive or
      // printf("XOR\n");
      rscv_reg->rg[rd] = rscv_reg->rg[rs1]^rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
    case 5: // SRL, SRA
      // printf("SRL/SRA\n");
      if (funct7 == 0) {
        rscv_reg->rg[rd] = rscv_reg->rg[rs1] >> rscv_reg->rg[rs2];
        rscv_reg->PC += 4;
      } else {
        rscv_reg->rg[rd] = (int32_t)rscv_reg->rg[rs1] >> rscv_reg->rg[rs2];
        rscv_reg->PC += 4;
      }
      break;
    case 6: // OR
      // printf("OR\n");
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] | rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
    case 7: // AND
      // printf("AND\n");
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] & rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
  }
}

// 1100011
void type_B (uint32_t word, RiscvRegister_t* rscv_reg) {
  uint32_t rs1 = get_rs1(word);
  uint32_t rs2 = get_rs2(word);
  uint32_t funct3 = get_funct3(word);
  int32_t imm = get_imm_B(word);

  switch(funct3) {
    case 0: // BEQ
      // printf("BEQ\n");
      if (rscv_reg->rg[rs1] == rscv_reg->rg[rs2]) {
        rscv_reg->PC = rscv_reg->PC + imm;
      } else {
        rscv_reg->PC += 4;
      }
      break;
    case 1: // BNE
      // printf("BNE\n");
      if (rscv_reg->rg[rs1] != rscv_reg->rg[rs2]) {
        // printf("IMM=%u\n", imm);
        rscv_reg->PC = rscv_reg->PC + imm;
      } else {
        rscv_reg->PC += 4;
      }
      break;
      // Branch less tha Signed
    case 4: // BLT
      // printf("BLT\n");
      if ((int32_t)rscv_reg->rg[rs1] < (int32_t)rscv_reg->rg[rs2]) {
        rscv_reg->PC = rscv_reg->PC + imm;
      } else {
        rscv_reg->PC += 4;
      }
      break;
    case 5: // BGE
      // Branch Greater that or equal Signed
      // printf("BGE\n");
      if ((int32_t)rscv_reg->rg[rs1] >= (int32_t)rscv_reg->rg[rs2]) {
        rscv_reg->PC = rscv_reg->PC + imm;
      } else {
        rscv_reg->PC += 4;
      }
      break;
    case 6: // BLTU
      // Branch less than unsigned
      // printf("BLTU\n");
      if ((uint32_t) rscv_reg->rg[rs1] < (uint32_t) rscv_reg->rg[rs2]) {
        rscv_reg->PC = rscv_reg->PC + imm;
      } else {
        rscv_reg->PC += 4;
      }
      break;
    case 7: // BGEU
      // Branch greater than or equal unsigned
      // printf("BGEU\n");
      if ((uint32_t) rscv_reg->rg[rs1] >= (uint32_t) rscv_reg->rg[rs2]) {
        rscv_reg->PC = rscv_reg->PC + imm;
      } else {
        rscv_reg->PC += 4;
      }
      break;
  };
}

// 0000011
void type_I (uint32_t word, RiscvRegister_t* rscv_reg, struct memory *mem) {
  uint32_t rs1 = get_rs1(word);
  uint32_t rd = get_rd(word);
  uint32_t funct3 = get_funct3(word);

  int32_t imm = get_imm_I(word);
  int32_t address = rscv_reg->rg[rs1] + imm;
  
  int8_t byte;
  int16_t halfword;
  int32_t full_word;
  uint8_t u_byte;
  uint16_t u_halfword;

  switch (funct3) {
    case 0: // LB
      // Load Byte
      // printf("LB\n");
      byte = memory_rd_b(mem, address);
      rscv_reg->rg[rd] = (int32_t)byte;
      rscv_reg->PC += 4;
      break;
    case 1: // LH
      // printf("LH\n");
      halfword = memory_rd_h(mem, address);
      rscv_reg->rg[rd] = (int32_t)halfword;
      rscv_reg->PC += 4;
      break;
    case 2: // LW
      // printf("LW\n");
      full_word = memory_rd_w(mem, address);
      rscv_reg->rg[rd] = (int32_t) full_word;
      rscv_reg->PC += 4;
      break;
    case 4: // LBU
      // printf("LBU\n");
      u_byte = memory_rd_b(mem, address);
      rscv_reg->rg[rd] = (uint32_t)u_byte;
      rscv_reg->PC += 4;
      break;
    case 5: // LHU
      // printf("LHU\n");
      u_halfword = memory_rd_h(mem, address);
      rscv_reg->rg[rd] = (uint32_t)u_halfword;
      rscv_reg->PC += 4;
      break;
  }
}

//0010011
void type_I2 (uint32_t word, RiscvRegister_t* rscv_reg) {
  uint32_t funct7 = get_funct7(word);
  uint32_t funct3 = get_funct3(word);
  uint32_t rd = get_rd(word);
  uint32_t rs1 = get_rs1(word);
  int32_t imm = get_imm_I(word);

  // Debug
  // printf("rs1: %u\n", rs1);
  // printf("imm_I: %i\n", imm);
  // printf("rd: %u\n", rd);

  switch (funct3) {
    case 0: // ADDI
      // printf("ADDI\n"); // I-type
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] + get_imm_I(word);
      rscv_reg->PC += 4;
      // printf("R[RS1]==%i\n", rscv_reg->rg[rs1]);
      // printf("R[RD]==%i\n", rscv_reg->rg[rd]);
      break;
    case 2: // SLTI
      // Set less than immediate signed
      // printf("SLTI\n");
      if ((int32_t) rscv_reg->rg[rs1] < imm) {
        rscv_reg->rg[rd] = 1;
        rscv_reg->PC += 4;
      } else {
        rscv_reg->rg[rd] = 0;
        rscv_reg->PC += 4;
      }
      break;
    case 3: // SLTIU
      // Set less than immediate unsigned
      // printf("SLTIU\n");
      if ((uint32_t) rscv_reg->rg[rs1] < (uint32_t) imm) {
        rscv_reg->rg[rd] = 1;
        rscv_reg->PC += 4;
      } else {
        rscv_reg->rg[rd] = 0;
        rscv_reg->PC += 4;
      }
      break;
    case 4: // XORI
      // Exclusive or immediate
      // printf("XORI\n");
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] ^ imm;
      rscv_reg->PC += 4;
      break;
    case 6: // ORI
      // printf("ORI\n"); // I-type
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] | get_imm_I(word);
      rscv_reg->PC += 4;
      break;
    case 7: // ANDI
      // printf("ANDI\n");
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] & imm;
      rscv_reg->PC += 4;
      break;
    
    // Next ones are different types with the shamt
    case 1: // SLLI
      // Shift left immidiate
      // printf("SLLI\n");
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] << imm;
      rscv_reg->PC += 4;
      break;
    case 5: // SRLI, SRAI
      // printf("SRLI/SRAI\n");
      if (funct7 == 0) {
        // printf("SRLI\n");
        rscv_reg->rg[rd] = rscv_reg->rg[rs1] >> imm;
        rscv_reg->PC += 4;
      } else {
        // printf("SRAI\n");
        rscv_reg->rg[rd] = (int32_t)rscv_reg->rg[rs1] >> imm;
        rscv_reg->PC += 4;
      }
      break;
  }
}

//0100011
void type_S (uint32_t word, RiscvRegister_t* rscv_reg, struct memory *mem) {
  uint32_t funct3 = get_funct3(word);
  uint32_t rs1 = get_rs1(word);
  uint32_t rs2 = get_rs2(word);
  int32_t imm = get_imm_S(word);

  int32_t address = rscv_reg->rg[rs1] + imm;

  int8_t byte;
  int16_t halfword;
  int32_t full_word;

  switch (funct3) {
    case 0: // SB
      // Store byte in memory
      // printf("SB\n");
      byte = rscv_reg->rg[rs2];
      // printf("SB Adress == %u\n",address );
      memory_wr_b(mem, address, byte);
      rscv_reg->PC += 4;
      break;
    case 1: // SH
      // store halfword in memory 
      // printf("SH\n");
      halfword = rscv_reg->rg[rs2];
      // printf("SH Adress == %u\n",address );
      memory_wr_h(mem, address, halfword);
      rscv_reg->PC += 4;
      break;
    case 2: // SW
      // store word in memory 
      // printf("SW\n");
      full_word = rscv_reg->rg[rs2];
      // printf("SW Adress == %0x, IMM== %i, RS1==%i, RS2==%i, R[RS1]==%i \n",address, imm, rs1, rs2, rscv_reg->rg[rs1] );
      memory_wr_w(mem, address, full_word);
      rscv_reg->PC += 4;
      break;
  }
}

