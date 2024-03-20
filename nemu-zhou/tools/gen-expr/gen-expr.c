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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#define _GEN_TIME_ 100

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"#include <stdint.h>\n"
"int main() { "
"  uint32_t result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static uint32_t gen_time = 5;
static uint32_t buf_p = 0;
static uint32_t num = 0;
static void gen_rand_op() {
	switch (rand()%4) {
		case 0:	
			buf[buf_p++] = ' ';
			buf[buf_p++] = '+'; 
			buf[buf_p++] = ' ';
			break;
		case 1:	
			buf[buf_p++] = ' ';
			buf[buf_p++] = '-'; 
			buf[buf_p++] = ' ';
			break;
		case 2:	
			buf[buf_p++] = ' ';
			buf[buf_p++] = '*'; 
			buf[buf_p++] = ' ';
			break;
		case 3:
			buf[buf_p++] = ' ';
			buf[buf_p++] = '/';
			buf[buf_p++] = ' ';
			num = random();
			buf[buf_p++] = '0';
			buf[buf_p++] = 'x';
			buf[buf_p++] = (num >> 28) < 10 ? (num >> 28) + '0' : (num >> 28) + 'a' - 10;
			buf[buf_p++] = ((num >> 24) & 0x0f) < 10 ? ((num >> 24) & 0x0f) + '0' : ((num >> 24) & 0x0f) + 'a' - 10;
			buf[buf_p++] = ((num >> 20) & 0x0f) < 10 ? ((num >> 20) & 0x0f) + '0' : ((num >> 20) & 0x0f) + 'a' - 10;
			buf[buf_p++] = ((num >> 16) & 0x0f) < 10 ? ((num >> 16) & 0x0f) + '0' : ((num >> 16) & 0x0f) + 'a' - 10;
			buf[buf_p++] = ((num >> 12) & 0x0f) < 10 ? ((num >> 12) & 0x0f) + '0' : ((num >> 12) & 0x0f) + 'a' - 10;
			buf[buf_p++] = ((num >> 8) & 0x0f) < 10 ? ((num >> 8) & 0x0f) + '0' : ((num >> 8) & 0x0f) + 'a' - 10;
			buf[buf_p++] = ((num >> 4) & 0x0f) < 10 ? ((num >> 4) & 0x0f) + '0' : ((num >> 4) & 0x0f) + 'a' - 10;
			buf[buf_p++] = 1 + 
				( ((num >> 0) & 0x0e) < 10 ? 
				  ((num >> 0) & 0x0e) + '0' : 
				  ((num >> 0) & 0x0e) + 'a' - 10 
				);
			buf[buf_p++] = 'u';
			buf[buf_p++] = ' ';
			buf[buf_p++] = '+';
			buf[buf_p++] = ' ';

	}
}
static void gen_rand_expr() {
	if (gen_time > 1) gen_time--;
	switch (rand() % gen_time) {
		case 0: 
			buf[buf_p++] = ' ';
			num = random();
			// printf("0x%08x\n", num);
			buf[buf_p++] = '0';
			buf[buf_p++] = 'x';
			buf[buf_p++] = (num >> 28) < 10 ? (num >> 28) + '0' : (num >> 28) + 'a' - 10;
			buf[buf_p++] = ((num >> 24) & 0x0f) < 10 ? ((num >> 24) & 0x0f) + '0' : ((num >> 24) & 0x0f) + 'a' - 10;
			buf[buf_p++] = ((num >> 20) & 0x0f) < 10 ? ((num >> 20) & 0x0f) + '0' : ((num >> 20) & 0x0f) + 'a' - 10;
			buf[buf_p++] = ((num >> 16) & 0x0f) < 10 ? ((num >> 16) & 0x0f) + '0' : ((num >> 16) & 0x0f) + 'a' - 10;
			buf[buf_p++] = ((num >> 12) & 0x0f) < 10 ? ((num >> 12) & 0x0f) + '0' : ((num >> 12) & 0x0f) + 'a' - 10;
			buf[buf_p++] = ((num >> 8) & 0x0f) < 10 ? ((num >> 8) & 0x0f) + '0' : ((num >> 8) & 0x0f) + 'a' - 10;
			buf[buf_p++] = ((num >> 4) & 0x0f) < 10 ? ((num >> 4) & 0x0f) + '0' : ((num >> 4) & 0x0f) + 'a' - 10;
			buf[buf_p++] = ((num >> 0) & 0x0f) < 10 ? ((num >> 0) & 0x0f) + '0' : ((num >> 0) & 0x0f) + 'a' - 10;
			buf[buf_p++] = 'u';
			buf[buf_p++] = ' ';
			break;
		case 1: 
			buf[buf_p++] = '(';
			gen_rand_expr();
			buf[buf_p++] = ')';

			break;
		default: 
			gen_rand_expr();
			gen_rand_op();
			gen_rand_expr();
			break;
	}
  buf[buf_p] = '\0';
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
	  buf_p = 0;
	  gen_time = _GEN_TIME_;
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc tmp/.code.c -o tmp/.expr");
    if (ret != 0) continue;

    fp = popen("tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("0x%08x %s\n", result, buf);
  }
  return 0;
}
