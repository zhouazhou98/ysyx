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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_hello(char *args) {
  printf("Hello NEMU!\n");
  return 0;
}

// 0~1 arg
static int cmd_si(char *args) {
  int max_argc = 1;
  printf("args: %s\n", args);
  if (args == NULL) {
    cpu_exec(1);
    return 0;
  }

  char * p = args;
  char * arg[max_argc];
  int i = 0;
  for (; (p != NULL) && (i < max_argc); i++) {
    arg[i] = strtok(p, " ");
    //printf("arg[%d]: %s\n", i, arg[i]/*, strlen(arg[i])*/ );
    p = p + strlen(arg[i]) + 1;
  }
  assert(i <= max_argc);
  char * argend = arg[0]  + strlen(arg[0] + 1);
  cpu_exec( strtoul(arg[0], &argend, 0) );
 
  return 0;
}

void print_wp();
static int cmd_info(char *args) {
  if(args == NULL) {
    printf("Need arg: \n\t`info r`: display register info\n\t`info w`: diaplay watchpoint info\n");
    return 0;
  }
  assert(strlen(strtok(args, " ")) == 1);
  switch (args[0]) {
    case 'r': isa_reg_display(); break;
    case 'w': print_wp(); break;
    
    default: printf("Need arg: \n\tr: display register info\n\tw:diaplay watchpoint info\n"); break;
  }
  return 0;
}

static int cmd_x(char *args) {
  int max_argc = 2;
  char * p = args;
  char * arg[max_argc];
  int i = 0;
  for (; (p != NULL) && (i < max_argc); i++) {
    arg[i] = strtok(p, " ");
    printf("arg[%d]: %s\n", i, arg[i]/*, strlen(arg[i])*/ );
    p = p + strlen(arg[i]) + 1;
  }
  //char * argend = arg[0]  + strlen(arg[0] + 1);
  char * arg0_end = arg[0] + strlen(arg[0]) + 1;
  uint64_t byte_num = strtoul( arg[0], &arg0_end, 0 );

  char * arg1_end = arg[1] + strlen(arg[1]) + 1;
  uint64_t vaddr = strtoul( arg[1], &arg1_end, 0 );
  printf("cmd x args:\n\targ1(display number of mem byte): %ld\n\targ2(display memory address): 0x%08lx\n", byte_num, vaddr);
  #include "memory/vaddr.h"
  for (int i = 0; i < byte_num; i++) {
    if ((i % 16) == 0)printf("\n%08lx: ", vaddr + i);
    uint8_t word = vaddr_read(vaddr + i, 1) & 0xff;
    printf("%02x ", word);
  }
  printf("\n");
  return 0;
}

static int cmd_p(char * args) {
  bool succ;
  word_t result = expr(args, &succ);
  printf("result: 0x%08x\n", result);
  if (succ) return result;
  return 0;
}

static char tb_result_cmd_p[12];
bool tb_cmd_p(char * args, int i, int total) {
  strncpy(tb_result_cmd_p, args, 11);
  tb_result_cmd_p[11] = '\0';
  char * tb_result_cmd_p_end = tb_result_cmd_p + 11;
  word_t result = strtoul(tb_result_cmd_p, &tb_result_cmd_p_end, 0); // *(uint32_t *)args;
  //printf("args: %s\n", args);
  args = args + 11;
  //printf("args: %s\n", args);
  bool succ;
  word_t result_cmd_p = expr(args, &succ);
  if (result_cmd_p == result) {
    printf("\033[0;32m[%d/%d] result: 0x%08x, result_cmd_p: 0x%08x\033[0m\n", i, total, result, result_cmd_p);
    return true;
  } else {
    printf("\033[0;31m[%d/%d] result: 0x%08x, result_cmd_p: 0x%08x\033[0m\n", i, total, result, result_cmd_p);
    return false;
  }
}

static int tb_p (char * args) {
  // char * tb_expr_l= "0x00000463 ( 0x4  / 0x5 + ( 0xb  + (((((( 0x3  + ( 0x1 ) +  0xb ) +  0xe  +  0xb ) +  0xc  +  0x8  / 0xf +  0x9  +  0x1  +  0xd  / 0x9 +  0x3  / 0xb +  0x3  +  0xb  +  0xf )) +  0x1  +  0xc  +  0xb  / 0x7 +  0xd  +  0x3  +  0xd  / 0x7 +  0xa  / 0xd +  0x7  / 0x5 +  0x6  +  0x7 )) +  0x3  +  0xe  +  0xa  +  0xa  +  0xe  +  0x5  / 0x1 +  0xf  +  0x6  +  0xc  / 0x7 +  0xc  +  0x2  / 0x5 +  0x9  +  0xc  +  0x4  / 0x1 +  0xd  +  0x1  / 0xd +  0x9  +  0x5  / 0x9 +  0xc ) +  0xe  +  0xd  +  0x6  +  0xc  +  0xd  +  0x1  +  0x4  +  0xd  +  0x2  +  0xa  +  0x8  / 0xf +  0xc  +  0x2  +  0xf  / 0x7 +  0xb  +  0x0  +  0x2  / 0x1 +  0x1  +  0x7  +  0x1  +  0x0  +  0xc  +  0x1  +  0x4  +  0x0  +  0x0  +  0x2  +  0xa  +  0x7  +  0x6  +  0x5  +  0x0  +  0xd  +  0xa  +  0x3  +  0xa  +  0x3  +  0xd  +  0x9  +  0x2  +  0x0  / 0x7 +  0x3  +  0x1  +  0xb  +  0xe  +  0x8  / 0x7 +  0x9  +  0xa  / 0x1 +  0x1  / 0x1 +  0x4  +  0x3  / 0x3 +  0x8  / 0x7 +  0x1  +  0x0  +  0x3  +  0xc  +  0x6  +  0x8  +  0x9  +  0xd ) +  0x4  +  0xd  +  0x0  / 0x9 +  0x3  +  0x6  +  0x3  +  0x9  +  0x2  +  0xb  +  0x8  +  0xc  +  0x9  +  0xa  +  0xc  +  0x7  / 0xd +  0x6  +  0x0  +  0x2  +  0xd  / 0x9 +  0x4  +  0x8  +  0xe  +  0x8  +  0x5  +  0x8  / 0x1 +  0x9  +  0xe  +  0x8  +  0x3  +  0x8  +  0x0  +  0xf  / 0x5 +  0x2  +  0x7  +  0xa  +  0x4  +  0x2  +  0xb  +  0xe  / 0x5 +  0xc  +  0xa  +  0x1  +  0x3  +  0xb  +  0x5  / 0x1 +  0x4  / 0xd +  0x7  / 0xd +  0x5  +  0xc  +  0x3  +  0x9  / 0xb +  0x7  +  0x2  +  0x7  +  0x1  +  0xd  +  0x4  +  0x9  +  0x5  +  0x8  / 0x3 +  0xa  +  0x1  / 0x7 +  0xa  / 0xb +  0x0  +  0x1  +  0x9  +  0xa  +  0x7  +  0xb  +  0xd  +  0xa  +  0x6  +  0x0  / 0x1 +  0x7  +  0x9  +  0x2  / 0xb +  0xa  +  0x6  +  0xa  +  0x0  / 0x7 +  0x2  +  0x8\0";
  FILE *file;
  char line[4096];
  file = fopen("expr/expr_input", "r");
  if (file == NULL) {
    printf("无法打开文件\n");
    return 1;
  }

  int tb_succ = 0;
  int tb_failed = 0;
  for (int i = 1; i <= 10000; i++){
    if (fgets(line, sizeof(line), file) != NULL) {
      //printf("line: %s\n", line);
      bool succ = tb_cmd_p(line, i, 10000);
      succ ? tb_succ++ : tb_failed++;
    } else {
      printf("error: readline return NULL");
      break;
    }
  }
  printf("\033[1;32msuccess: [%d/10000]\033[0m, \033[1;31msuccess: [%d/10000]\033[0m\n", tb_succ, tb_failed);

  return 0;
}


void add_wp(char * expr_str);
void del_wp(int no);

static int cmd_wp(char * args) {
  if(args == NULL) {
    printf("Need arg: \n\t`wp n [expr]`: new a watchpoint for expr [expr]\n\t`wp d num`: delete the `num`th watchpoint\n\t`info w`: diaplay watchpoint info\n");
    return 0;
  }
  assert(strlen(strtok(args, " ")) == 1);
  switch (args[0]) {
    case 'n':
      char * expr_str = args + 2;
      add_wp(expr_str);
      break;
    case 'd':
      char * no = args + 2;
      char * no_end = no;
      while (no_end[0] != '\0') no_end++;
      int num = strtol(no, &no_end, 0);
      del_wp(num);
      break;
    default: 
      printf("Need arg: \n\t`wp n [expr]`: new a watchpoint for expr [expr]\n\t`wp d num`: delete the `num`th watchpoint\n\t`info w`: diaplay watchpoint info\n");
      break;
  }
  return 0;
}


typedef struct cmd_table_t {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table_t;

static cmd_table_t cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "hello", "Hello NEMU", cmd_hello },
  { "si", "Next (num) Step Instruction", cmd_si},
  { "info", "difplay info (args: r)", cmd_info},
  { "x", "Display / Scan Memory", cmd_x},
  { "p", "print expr", cmd_p},
  { "wp", "watchpoint option", cmd_wp },
  { "tb_p", "testbench of cmd_p", tb_p},

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
