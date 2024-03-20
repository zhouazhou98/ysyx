/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum {
  TYPE_I, TYPE_U, TYPE_S, TYPE_J, TYPE_R, TYPE_B,
  TYPE_N, // none
};

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
#define immJ() do { *imm = SEXT( 	( BITS(i, 31, 31) 	  << 20		\
    					| ( BITS(i, 19, 12) 	  << 12)	\
    					| ( BITS(i, 20, 20)	  << 11)	\
    					| BITS(i, 30, 21)	  << 1)		\
					& 0xfffffffe				\
    				, 21); } while(0)
#define immB() do { *imm = SEXT( 	BITS(i, 31, 31) 	<< 11	\
					| BITS(i, 7, 7) 		<< 10	\
					| BITS(i, 30, 25)		<< 4	\
					| BITS(i, 11, 8)			\
		, 12); } while(0)

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd     = BITS(i, 11, 7);
  switch (type) {
    case TYPE_I: src1R();          immI(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
    case TYPE_J: 		   immJ(); break;
    case TYPE_R: src1R(); src2R();         break;
    case TYPE_B: src1R(); src2R(); immB(); break;
  }
  printf("\033[1;37;43mrd: 0x%08x, src1: 0x%08x, src2: 0x%08x, imm: 0x%08x.\033[0m\n", R(*rd), *src1, *src2, *imm);
}

static int decode_exec(Decode *s) {
  int rd = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}

  INSTPAT_START();
// 1. RV32I: TODO: ECALL, FENCE
  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi   , I, R(rd) = src1 + imm; printf("\033[1;37;40mrd(%d) = 0x%08x = src1 + imm = 0x%08x + 0x%08x.\033[0m\n", rd, R(rd), src1, imm););
  INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli   , I, R(rd) = src1 << (imm & 0b011111) );
  INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti   , I, R(rd) = ((int32_t)src1 < (int32_t)imm) ? 0x01 : 0x0 );			// Page 18
  INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu  , I, R(rd) = ( (word_t)src1 < (word_t)imm ) ? 0x01 : 0x0 );	// Page 18
  INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori   , I, R(rd) = src1 ^ imm);
  INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli   , I, R(rd) = (word_t)src1 >> (imm & 0b011111) );
  INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai   , I, do { R(rd) = (int32_t)src1 >> (imm & 0b011111) ; R(rd) = (src1 >> 31 == 0) ? R(rd) : (R(rd) | ( (0x1 << (imm & 0b011111) ) - 1 ) << (32 - (imm & 0b011111) )) ; printf("\033[1;37;40mrd(%d) = 0x%08x\033[0m\n", rd, R(rd) ); } while(0) );
  INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori    , I, R(rd) = src1 | imm);
  INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi   , I, R(rd) = src1 & imm);


  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add    , R, R(rd) = src1 + src2; printf("\033[1;37;40mrd(%d) = 0x%08x = src1 + src2 = 0x%08x + 0x%08x.\033[0m\n", rd, R(rd), src1, src2););
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub    , R, R(rd) = src1 - src2; printf("\033[1;37;40mrd(%d) = 0x%08x = src1 + src2 = 0x%08x - 0x%08x.\033[0m\n", rd, R(rd), src1, src2););
  INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll    , R, R(rd) = (word_t)src1 << (src2 & 0b011111) );
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt    , R, R(rd) = ((int32_t)src1 < (int32_t)src2) ? 0x01 : 0x0 );
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu   , R, R(rd) = ((word_t)src1 < (word_t)src2) ? 0x01 : 0x0 );
  INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor    , R, R(rd) = src1 ^ src2);
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl    , R, R(rd) = src1 >> (src2 & 0b011111) );
  INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra    , R, do { R(rd) = src1 >> (src2 & 0b011111) ; R(rd) = (src1 >> 31 == 0) ? R(rd) : (R(rd) | ( (0x1 << (src2 & 0b011111) ) - 1 ) << (32 - (src2 & 0b011111) )) ; } while(0) );
  INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or     , R, R(rd) = src1 | src2);
  INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and    , R, R(rd) = src1 & src2);

  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(rd) = s->pc + imm; printf("\033[1;37;40mrd(%d) = 0x%08x = pc + imm = 0x%08x + 0x%08x.\033[0m\n", rd, R(rd), s->pc, imm););

  INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq	   , B, if (src1 == src2) { ( s->dnpc = s->pc + (imm << 1) ) ; } ; printf("\033[1;37;40mnow pc = 0x%08x, next pc = 0x%08x\033[0m\n", s->pc, s->dnpc); );
  INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne	   , B, if (src1 != src2) { ( s->dnpc = s->pc + (imm << 1) ) ; } ; printf("\033[1;37;40mnow pc = 0x%08x, next pc = 0x%08x\033[0m\n", s->pc, s->dnpc); );
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt	   , B, if ((int32_t)src1 < (int32_t)src2) { ( s->dnpc = s->pc + (imm << 1) ); } ; printf("\033[1;37;40mnow pc = 0x%08x, next pc = 0x%08x\033[0m\n", s->pc, s->dnpc); );
  INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge	   , B, if ((int32_t)src1 >= (int32_t)src2) {  s->dnpc = s->pc + (imm << 1); } ; printf("\033[1;37;40mnow pc = 0x%08x, next pc = 0x%08x\033[0m\n", s->pc, s->dnpc); );
  INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu   , B, if ((word_t)src1 < (word_t)src2) { ( s->dnpc = s->pc + (imm << 1) ); } ; printf("\033[1;37;40mnow pc = 0x%08x, next pc = 0x%08x\033[0m\n", s->pc, s->dnpc); );
  INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu   , B, if ((word_t)src1 >= (word_t)src2) {  s->dnpc = s->pc + (imm << 1) ; } ; printf("\033[1;37;40mnow pc = 0x%08x, next pc = 0x%08x\033[0m\n", s->pc, s->dnpc); );

  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal    , J, ( R(rd) = s->pc + 4); ( s->dnpc = s->pc + imm ); printf("\033[1;37;40mrd(%d) = 0x%08x = pc + 4 = 0x%08x + 4;\tNext pc = 0x%08x = pc + imm = 0x%08x + 0x%08x\033[0m\n", rd, R(rd), s->pc, s->dnpc, s->pc, imm););

  INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr   , I, ( R(rd) = s->pc + 4); ( s->dnpc = (imm + src1) & 0xfffffffe ); printf("\033[1;37;40mrd(%d) = 0x%08x = pc + 4 = 0x%08x + 4;\tnext pc = 0x%08x = src1 + imm = 0x%08x + 0x%08x\033[0m\n", rd, R(rd), s->pc, s->dnpc, src1, imm););

  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(rd) = imm; printf("\033[1;37;40mrd(%d) = 0x%08x = imm.\033[0m\n", rd, imm); );

  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2); printf("\033[1;37;40mstore 1 byte (0x%02x) to memory(0x%08x)\033[0m\n", src2, src1 + imm); );
  INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh     , S, Mw(src1 + imm, 2, src2); printf("\033[1;37;40mstore 2 byte (0x%04x) to memory(0x%08x)\033[0m\n", src2, src1 + imm); );
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, Mw(src1 + imm, 4, src2); printf("\033[1;37;40mstore 4 byte (0x%08x) to memory(0x%08x)\033[0m\n", src2, src1 + imm); );

  INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb     , I, R(rd) = SEXT( Mr(src1 + imm, 1), 8); printf("\033[1;37;40mread 1 byte (0x%08x)(signed-extended) from memory(0x%08x) to register(%d).\033[0m\n", R(rd), src1 + imm, rd); );
  INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh     , I, R(rd) = SEXT( Mr(src1 + imm, 2), 16); printf("\033[1;37;40mread 2 byte (0x%08x)(signed-extended) from memory(0x%08x) to register(%d).\033[0m\n", R(rd), src1 + imm, rd); );
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, R(rd) = Mr(src1 + imm, 4); printf("\033[1;37;40mread 4 byte (0x%08x)(signed-extended) from memory(0x%08x) to register(%d).\033[0m\n", R(rd), src1 + imm, rd); );
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(rd) = 0xff & Mr(src1 + imm, 1); printf("\033[1;37;40mread 1 byte (0x%08x)(0-extended) from memory(0x%08x) to register(%d).\033[0m\n", R(rd), src1 + imm, rd); );
  INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu    , I, R(rd) = 0xffff & Mr(src1 + imm, 2); printf("\033[1;37;40mread 2 byte (0x%08x)(0-extended) from memory(0x%08x) to register(%d).\033[0m\n", R(rd), src1 + imm, rd); );

  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0

// 2. RV32M
  uint64_t rv32m_temp = 0;
  uint64_t rv32m_a = 0;
  uint64_t rv32m_b = 0;
  INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul    , R, R(rd) = (src1 * src2) & 0xffffffff; printf("\033[1;37;43mrd: 0x%08x, src1: 0x%08x, src2: 0x%08x, imm: 0x%08x.\033[0m\n", R(rd), src1, src2, imm); );
  INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh   , R, rv32m_a = SEXT(src1, 32); rv32m_b = SEXT(src2, 32); rv32m_temp = ( (int64_t)rv32m_a * (int64_t)rv32m_b ); R(rd) =(word_t)(rv32m_temp >> 32); printf("\033[1;37;43mrd: 0x%08x, src1: 0x%08x, src2: 0x%08x, imm: 0x%08x.\033[0m\n", R(rd), src1, src2, imm); );
  INSTPAT("0000001 ????? ????? 010 ????? 01100 11", mulhsu , R, rv32m_a = SEXT(src1, 32); rv32m_b = src2; rv32m_temp = ( rv32m_a * rv32m_b ); R(rd) = rv32m_temp >> 32; printf("rv32m_temp: 0x%16lx\n", rv32m_temp); );
  INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulhu  , R, rv32m_a = src1; rv32m_b = src2; rv32m_temp = ( (uint64_t)rv32m_a * (uint64_t)rv32m_b  ); R(rd) = rv32m_temp >> 32; printf("rv32m_temp: 0x%16lx\n", rv32m_temp); );
  INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div    , R, R(rd) = (int32_t)src1 / (int32_t)src2; );
  INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu   , R, R(rd) = (word_t)src1  / (word_t)src2;  );
  INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem    , R, R(rd) = (int32_t)src1 % (int32_t)src2; if (src1 >> 31) { R(rd) = 0x80000000 | R(rd) ; } );
  INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu   , R, R(rd) = (word_t)src1 % (word_t)src2;   );


  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
  INSTPAT_END();
  

  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}