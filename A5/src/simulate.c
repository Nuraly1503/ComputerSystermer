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

  // log sdfsdf
  struct Log log;

  while (1) {
    // Read word (instruction set)
    word = memory_rd_w(mem, rscv_reg.PC);

    // Increase instruction count
    inst_cnt++;

    // log
    log.addr = rscv_reg.PC;

    // Decode instruction set
    opcode = get_opcode(word);
    rs1 = get_rs1(word); 
    rd = get_rd(word);
    funct7 = get_funct7(word);
    rg_rs1 = rscv_reg.rg[rs1];
    ecall_val = rscv_reg.rg[a7];
    rscv_reg.rg[0] = 0; // x0 should always be zero
  

    switch(opcode) {
      case 55: // LUI
        log.instr = "lui";
        rscv_reg.rg[rd] = get_imm_U(word);
        rscv_reg.PC += 4;
        break;
      case 23: // AUIPC
        log.instr = "auipc";
        rscv_reg.rg[rd] = rscv_reg.PC + get_imm_U(word);
        rscv_reg.PC += 4;
        break;
      case 111: // JAL
        log.instr = "jal";
        rscv_reg.rg[rd] = rscv_reg.PC + 4;
        rscv_reg.PC = rscv_reg.PC + get_imm_J(word);
        break;
      case 103: // JALR
        log.instr = "jalr";
        rscv_reg.rg[rd] = rscv_reg.PC + 4;
        rscv_reg.PC = rg_rs1 + get_imm_I(word);
        break;
      case TYPE_B:  
        type_B(word, &rscv_reg, &log);
        break;
      case TYPE_R:
        // helper_extension(word); // helper_type_R
        // To differentiate between ex or type_R
        switch (funct7) {
          case 0:
            type_R(word, &rscv_reg, &log);
            break;
          case 32:
            type_R(word, &rscv_reg, &log);
            break;
          case 1:
            helper_extension(word, &rscv_reg, &log);
            break;
        }
        break;
      case TYPE_I:
        type_I(word, &rscv_reg, mem, &log);
        break;
      case TYPE_I2:
        type_I2(word, &rscv_reg, &log);
        break;
      case TYPE_S:
        type_S(word, &rscv_reg, mem, &log);
        break;
      
      case ECALL:
        log.instr = "ecall";
        switch(ecall_val) {
          case GETCHAR:
            // returner "getchar()" i A0
            rscv_reg.rg[a0] = getchar();
            rscv_reg.PC += 4;
            break;
          case PUTCHAR:
            // udfør "putchar(c)", hvor c tages fra A0
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

    // Write to log
    if (log_file != NULL) {
      char str[1000];
      sprintf(str, "%5llu %10x : %08x ", inst_cnt, log.addr, word);
      if (strcmp(assembly_get(as, log.addr), "") == 0) {
        assembly_set(as, log.addr, log.instr);
      }
      fprintf(log_file, "%s %s\n", str, assembly_get(as, log.addr));
    }

  }

  return inst_cnt;
}


/* 
 * Immidiate Encoding Variants helper functions 
 */

// Mash together type_R
//0110011
void helper_extension (uint32_t word, RiscvRegister_t* rscv_reg, Log_t* log) {
  uint32_t rs1 = get_rs1(word);
  uint32_t rs2 = get_rs2(word);
  uint32_t rd = get_rd(word);
  uint32_t funct3 = get_funct3(word);

  switch (funct3) {
    case 0: // MUL
      // Multiply two signed numbers
      log->instr = "mul";
      rscv_reg-> rg[rd] = (int32_t)rscv_reg->rg[rs1] * (int32_t)rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
    case 1: // MULH
      log->instr = "mulh";
      rscv_reg-> rg[rd] = ((int64_t) rscv_reg->rg[rs1] * (int64_t) rscv_reg->rg[rs2]) >> 32;
      rscv_reg->PC += 4;
      break;
    case 2: // MULHSU
      // One signed with one unsigned
      log->instr = "mulhsu";
      rscv_reg-> rg[rd] = ((int64_t) rscv_reg->rg[rs1] * (uint64_t) rscv_reg->rg[rs2]) >> 32;
      rscv_reg->PC += 4;
      break;
    case 3: // MULHU
      // Multiply two unsigned numbers
      log->instr = "mulhu";
      rscv_reg-> rg[rd] = ((uint64_t) rscv_reg->rg[rs1] * (uint64_t) rscv_reg->rg[rs2]) >> 32;
      rscv_reg->PC += 4;
      break;
    case 4: // DIV 
      // Divide two signed integers
      log->instr = "div";
      rscv_reg-> rg[rd] = (int32_t) rscv_reg->rg[rs1] / (int32_t)rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
    case 5: //DIVU
      // Divide two unsigned integers
      log->instr = "divu";
      rscv_reg-> rg[rd] = (uint32_t) rscv_reg->rg[rs1] / (uint32_t) rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
    case 6: // REM
      // Modulo of two signed integers
      log->instr = "rem";
      rscv_reg-> rg[rd] = (int32_t) rscv_reg->rg[rs1] % (int32_t)rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
    case 7: // REMU
      // Modulo of two unsigned integers
      log->instr = "remu";
      rscv_reg-> rg[rd] = (uint32_t) rscv_reg->rg[rs1] % (uint32_t) rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
  }
}

//0110011
void type_R (uint32_t word, RiscvRegister_t* rscv_reg, Log_t* log) {
  uint32_t rs2 = get_rs2(word);
  uint32_t rd = get_rd(word);
  uint32_t rs1 = get_rs1(word);
  uint32_t funct3 = get_funct3(word);
  uint32_t funct7 = get_funct7(word);

  switch (funct3) {
    case 0: // ADD, SUB
      if (funct7 == 0) {
        log->instr = "add";
        rscv_reg->rg[rd] = rscv_reg->rg[rs1] + rscv_reg->rg[rs2];
        rscv_reg->PC += 4;
      } else {
        log->instr = "sub";
        rscv_reg->rg[rd] = rscv_reg->rg[rs1] - rscv_reg->rg[rs2];
        rscv_reg->PC += 4;
      }
      break;
    case 1: // SLL
      log->instr = "sll";
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] << rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
    case 2: // SLT
      // set less than if rs1 < rs2 else 0
      log->instr = "slt";
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
      log->instr = "sltu";
      if ((uint32_t) rscv_reg->rg[rs1] < (uint32_t) rscv_reg->rg[rs2]) {
        rscv_reg->rg[rd] = 1;
        rscv_reg->PC += 4;
      } else {
        rscv_reg->rg[rd] = 0;
        rscv_reg->PC += 4;
      }
      break;
    case 4: // XOR
      // Exclusive or
      log->instr = "xor";
      rscv_reg->rg[rd] = rscv_reg->rg[rs1]^rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
    case 5: // SRL, SRA
      if (funct7 == 0) {
        log->instr = "srl";
        rscv_reg->rg[rd] = rscv_reg->rg[rs1] >> rscv_reg->rg[rs2];
        rscv_reg->PC += 4;
      } else {
        log->instr = "sra";
        rscv_reg->rg[rd] = (int32_t)rscv_reg->rg[rs1] >> rscv_reg->rg[rs2];
        rscv_reg->PC += 4;
      }
      break;
    case 6: // OR
      log->instr = "or";
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] | rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
    case 7: // AND
      log->instr = "and";
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] & rscv_reg->rg[rs2];
      rscv_reg->PC += 4;
      break;
  }
}

// 1100011
void type_B (uint32_t word, RiscvRegister_t* rscv_reg, Log_t* log) {
  uint32_t rs1 = get_rs1(word);
  uint32_t rs2 = get_rs2(word);
  uint32_t funct3 = get_funct3(word);
  int32_t imm = get_imm_B(word);

  switch(funct3) {
    case 0: // BEQ
      log->instr = "beq";
      if (rscv_reg->rg[rs1] == rscv_reg->rg[rs2]) {
        rscv_reg->PC = rscv_reg->PC + imm;
      } else {
        rscv_reg->PC += 4;
      }
      break;
    case 1: // BNE
      log->instr = "bne";
      if (rscv_reg->rg[rs1] != rscv_reg->rg[rs2]) {
        rscv_reg->PC = rscv_reg->PC + imm;
      } else {
        rscv_reg->PC += 4;
      }
      break;
      // Branch less tha Signed
    case 4: // BLT
      log->instr = "blt";
      if ((int32_t)rscv_reg->rg[rs1] < (int32_t)rscv_reg->rg[rs2]) {
        rscv_reg->PC = rscv_reg->PC + imm;
      } else {
        rscv_reg->PC += 4;
      }
      break;
    case 5: // BGE
      // Branch Greater that or equal Signed
      log->instr = "bge";
      if ((int32_t)rscv_reg->rg[rs1] >= (int32_t)rscv_reg->rg[rs2]) {
        rscv_reg->PC = rscv_reg->PC + imm;
      } else {
        rscv_reg->PC += 4;
      }
      break;
    case 6: // BLTU
      // Branch less than unsigned
      log->instr = "bltu";
      if ((uint32_t) rscv_reg->rg[rs1] < (uint32_t) rscv_reg->rg[rs2]) {
        rscv_reg->PC = rscv_reg->PC + imm;
      } else {
        rscv_reg->PC += 4;
      }
      break;
    case 7: // BGEU
      // Branch greater than or equal unsigned
      log->instr = "bgeu";
      if ((uint32_t) rscv_reg->rg[rs1] >= (uint32_t) rscv_reg->rg[rs2]) {
        rscv_reg->PC = rscv_reg->PC + imm;
      } else {
        rscv_reg->PC += 4;
      }
      break;
  };
}

// 0000011
void type_I (uint32_t word, RiscvRegister_t* rscv_reg, struct memory *mem, Log_t* log) {
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
      log->instr = "lb";
      byte = memory_rd_b(mem, address);
      rscv_reg->rg[rd] = (int32_t)byte;
      rscv_reg->PC += 4;
      break;
    case 1: // LH
      log->instr = "lh";
      halfword = memory_rd_h(mem, address);
      rscv_reg->rg[rd] = (int32_t)halfword;
      rscv_reg->PC += 4;
      break;
    case 2: // LW
      log->instr = "lw";
      full_word = memory_rd_w(mem, address);
      rscv_reg->rg[rd] = (int32_t) full_word;
      rscv_reg->PC += 4;
      break;
    case 4: // LBU
      log->instr = "lbu";
      u_byte = memory_rd_b(mem, address);
      rscv_reg->rg[rd] = (uint32_t)u_byte;
      rscv_reg->PC += 4;
      break;
    case 5: // LHU
      log->instr = "lhu";
      u_halfword = memory_rd_h(mem, address);
      rscv_reg->rg[rd] = (uint32_t)u_halfword;
      rscv_reg->PC += 4;
      break;
  }
}

//0010011
void type_I2 (uint32_t word, RiscvRegister_t* rscv_reg, Log_t* log) {
  uint32_t funct7 = get_funct7(word);
  uint32_t funct3 = get_funct3(word);
  uint32_t rd = get_rd(word);
  uint32_t rs1 = get_rs1(word);
  int32_t imm = get_imm_I(word);
  int32_t shamt = get_shamt(word);

  switch (funct3) {
    case 0: // ADDI
      log->instr = "addi";
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] + get_imm_I(word);
      rscv_reg->PC += 4;
      break;
    case 2: // SLTI
      // Set less than immediate signed
      log->instr = "slti";
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
      log->instr = "sltiu";
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
      log->instr = "xori";
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] ^ imm;
      rscv_reg->PC += 4;
      break;
    case 6: // ORI
      log->instr = "ori";
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] | get_imm_I(word);
      rscv_reg->PC += 4;
      break;
    case 7: // ANDI
      log->instr = "andi";
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] & imm;
      rscv_reg->PC += 4;
      break;
    
    // Next ones are different types with the shamt
    case 1: // SLLI
      // Shift left immidiate
      log->instr = "slli";
      rscv_reg->rg[rd] = rscv_reg->rg[rs1] << shamt;
      rscv_reg->PC += 4;
      break;
    case 5: // SRLI, SRAI
      if (funct7 == 0) {
        log->instr = "srli";
        rscv_reg->rg[rd] = rscv_reg->rg[rs1] >> shamt;
        rscv_reg->PC += 4;
      } else {
        log->instr = "srai";
        rscv_reg->rg[rd] = (int32_t)rscv_reg->rg[rs1] >> shamt;
        rscv_reg->PC += 4;
      }
      break;
  }
}

//0100011
void type_S (uint32_t word, RiscvRegister_t* rscv_reg, struct memory *mem, Log_t* log) {
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
      log->instr = "sb";
      byte = rscv_reg->rg[rs2];
      memory_wr_b(mem, address, byte);
      rscv_reg->PC += 4;
      break;
    case 1: // SH
      // store halfword in memory 
      log->instr = "sh";
      halfword = rscv_reg->rg[rs2];
      memory_wr_h(mem, address, halfword);
      rscv_reg->PC += 4;
      break;
    case 2: // SW
      // store word in memory 
      log->instr = "sw";
      full_word = rscv_reg->rg[rs2];
      memory_wr_w(mem, address, full_word);
      rscv_reg->PC += 4;
      break;
  }
}

