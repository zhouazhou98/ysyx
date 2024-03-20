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

#include <isa.h>
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};
/*
void print_binary(int n) {
    int i;
    for (i = 31; i >= 0; i--) {
        if (n & (1 << i)) {
            printf("1");
        } else {
            printf("0");
        }
    }
    printf("\n");
}

void isa_reg_display() {
  printf("R[pc]: %8x -->\t", cpu.pc);
  print_binary(cpu.pc);
  for (int i = 0; i < 32; i++) {
    printf("R[%s]: %08x -->\t", regs[i], cpu.gpr[i]);
    print_binary( cpu.gpr[i] );
  }
}
*/

void isa_reg_display() {
  printf("R[pc ]: %02x %02x %02x %02x\n", 0xff & (cpu.pc >> 24), 0xff & (cpu.pc >> 16), 0xff & (cpu.pc >> 8), 0xff & cpu.pc);
  for (int i = 0; i < 32; i++) {
    printf("R[%-*s]: ", 3, regs[i]);
    printf("%02x %02x %02x %02x", (cpu.gpr[i] >> 24) & 0xff, (cpu.gpr[i] >> 16) & 0xff, (cpu.gpr[i] >> 8) & 0xff, cpu.gpr[i] & 0xff);
    (i % 2 == 0) ? printf("\t") : printf("\n");
  }
}



// 寄存器映射结构体
typedef struct {
    char* regName;
    int regIndex;
} RegisterMapping;

// 初始化寄存器映射表
static RegisterMapping regMappings[] = {
    {"$x0", 0}, {"$zero", 0}, {"$0", 0}, {"$$0", 0},
    {"$x1", 1}, {"$ra", 1},
    {"$x2", 2}, {"$sp", 2},
    {"$x3", 3}, {"$gp", 3},
    {"$x4", 4}, {"$tp", 4},
    {"$x5", 5}, {"$t0", 5},
    {"$x6", 6}, {"$t1", 6},
    {"$x7", 7}, {"$t2", 7},
    {"$x8", 8}, {"$s0", 8},
    {"$x9", 9}, {"$s1", 9},
    {"$x10", 10}, {"$a0", 10},
    {"$x11", 11}, {"$a1", 11},
    {"$x12", 12}, {"$a2", 12},
    {"$x13", 13}, {"$a3", 13},
    {"$x14", 14}, {"$a4", 14},
    {"$x15", 15}, {"$a5", 15},
    {"$x16", 16}, {"$a6", 16},
    {"$x17", 17}, {"$a7", 17},
    {"$x18", 18}, {"$s2", 18},
    {"$x19", 19}, {"$s3", 19},
    {"$x20", 20}, {"$s4", 20},
    {"$x21", 21}, {"$s5", 21},
    {"$x22", 22}, {"$s6", 22},
    {"$x23", 23}, {"$s7", 23},
    {"$x24", 24}, {"$s8", 24},
    {"$x25", 25}, {"$s9", 25},
    {"$x26", 26}, {"$s10", 26},
    {"$x27", 27}, {"$s11", 27},
    {"$x28", 28}, {"$t3", 28},
    {"$x29", 29}, {"$t4", 29},
    {"$x30", 30}, {"$t5", 30},
    {"$x31", 31}, {"$t6", 31},
    {"$pc", 32}, {"pc", 32}, // 在此处添加其他寄存器的映射
};


word_t isa_reg_str2val(const char *s, bool *success) {
  for (int i = 0; i < sizeof(regMappings) / sizeof(regMappings[0]); i++) {
        if (strcmp(regMappings[i].regName, s) == 0) {
            if (success != NULL) {
                *success = true;
            }
            if ( strcmp(s, "$pc") == 0 || 0 == strcmp(s, "pc") ) return cpu.pc;
            return cpu.gpr[regMappings[i].regIndex];
        }
    }
  if (success != NULL) {
    *success = false;
  }
  return -1; // 如果没有匹配的寄存器名，返回-1表示未找到
}
