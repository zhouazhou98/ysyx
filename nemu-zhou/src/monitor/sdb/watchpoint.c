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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  uint32_t old_value;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char expr[4096];  // 其实也可以使用 expr.c 中的 tokens[] ，减少 make_token 步骤，不过这里为了让 expr.c 中的表达式尽量少的对外暴露，就使用字符串了

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].old_value = 0;
    wp_pool[i].expr[0] = '\0';
  }

  head = NULL;
  // 空闲 wp 链表
  free_ = wp_pool;
}

bool wp_si() {
  int i = 0;
  bool ret = true;
  for (WP * wp = head; wp != NULL; wp = wp->next, i++) {
    bool succ;
    uint32_t new_value = expr(wp->expr, &succ);
    if (new_value != wp->old_value) {
      printf("\033[0;33mwp [%2d]:\toldvalue = 0x%08x,\tnewvalue = 0x%08x,\n\t%s\033[0m\n", i, wp->old_value, new_value, wp->expr);
      wp->old_value = new_value;
      ret = false;
    }
  }
  return ret;
}

/* TODO: Implement the functionality of watchpoint */
void add_wp(char * expr_str) {
  WP * wp = free_;
  if (free_ != NULL) {
      free_ = free_->next;
      wp->next = head;
      head = wp;
      wp = NULL;
  } else {
    assert(0); // 只有 32 个 WP，申请不出来了
  }
  bool succ;
  head->old_value = expr(expr_str, &succ);
  // strncpy(head->expr, expr_str, strlen(expr_str));
  int i = 0;
  while (expr_str[i] != '\0') {
    head->expr[i] = expr_str[i];
    i++;
  }
  head->expr[i] = '\0';

} 
// no : 0 --> 31
void del_wp(int no) {
  assert(no > 0);
  assert(no < NR_WP);
  WP * wp = head;
  while (wp != NULL && (--no) ) wp = wp->next;
  if (wp == NULL || wp->next == NULL) printf("没有这个监视点！\n");
  WP * tmp = wp->next;
  // del wp from head
  wp->next = tmp->next;
  tmp->next = NULL;
  // add WP to free_
  tmp->next = free_;
  free_ = tmp;
  // reset WP
  tmp->old_value = 0;
  tmp->expr[0] = '\0';

}

void print_wp() {
  int i = 0;
  for (WP * wp = head; wp != NULL; wp = wp->next, i++) {
    printf("\033[0;33mwp [%2d]:\toldvalue = 0x%08x,\n\t%s\033[0m\n", i, wp->old_value, wp->expr);
  }
}

