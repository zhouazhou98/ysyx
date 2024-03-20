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
#include <stdint.h>
#include <assert.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_END,

  /* TODO: Add more token types */
  TK_NUM, TK_REG, TK_MEM, TK_NEG, TK_NL,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces 这里的 + 是对前面空格 ` ` 的正则扩展
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  
  {"\\-", '-'},         // sub
  {"\\*", '*'},         // mul
  {"\\/", '/'},         // div
  {"\\(", '('},         // 
  {"\\)", ')'},         // 
  {"(0b[01]+|0o[0-7]+|0x[0-9a-fA-F]+|[0-9]+)u?", TK_NUM},
  {"\\$(0|x[0-9]|x1[0-9]|x2[0-9]|x3[0-1]|zero|ra|sp|gp|tp|t[0-6]|s[0-9]|s1[0-1]|a[0-7]|pc)", TK_REG},
  {"\n", TK_NL},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}


#define STACK_DEPTH 127

typedef struct token {
  uint32_t type;
  char str[12];
} Token;

typedef struct __attribute__((aligned(4096))) {
    Token tokens[STACK_DEPTH];
    int32_t top;
} Stack;
typedef struct __attribute__((aligned(4096))) {
  uint32_t nums[STACK_DEPTH];
  int top;
} NumStack;

static Token tokens[1024] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;
static NumStack num_stack __attribute__((used)) ;
static Stack symb_stack __attribute__((used)) ;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;
  num_stack.top = -1;
  symb_stack.top = -1;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        //if (rules[i].token_type != TK_NOTYPE) Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //    i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        // // 加入树？？如果是 - 号，可以判断其前面一个匹配到的 token 类型是否为数字，若为开头 或 不是数字，则为 取负
        switch (rules[i].token_type) {
          case TK_NOTYPE: break;
          case '+':
          case '/':
          case '-':
          case TK_EQ:
          case TK_REG:
          case TK_NUM:
          case '(':
          case ')':
            tokens[nr_token].type = rules[i].token_type;
            strncpy( tokens[nr_token].str, substr_start, substr_len );
            tokens[nr_token].str[substr_len] = '\0';
            // printf("tokens[%d]: %s\n", nr_token, tokens[nr_token].str);
            nr_token++;
            break;
          case '*':
            //printf("tokens[nr_token-1].type: %d, %d\n", tokens[nr_token-1].type, TK_NUM);
            if (  (tokens[nr_token-1].type == TK_NUM) 
                  || (tokens[nr_token-1].type == TK_REG)
                  || (tokens[nr_token-1].type == ')') ) {
              tokens[nr_token].type = rules[i].token_type;
              strncpy( tokens[nr_token].str, substr_start, substr_len );
              tokens[nr_token].str[substr_len] = '\0';
              //printf("tokens[%d]: %s\n", nr_token, tokens[nr_token].str);
              nr_token++;
            } else {
              tokens[nr_token].type = TK_MEM;
              strncpy( tokens[nr_token].str, substr_start, substr_len );
              tokens[nr_token].str[substr_len] = '\0';
              //printf("tokens[%d]: %s\n", nr_token, tokens[nr_token].str);
              nr_token++;
            }
            break;
          case TK_NL:
            //printf("goto token_end!\n");
            goto token_end;
          
          default: 
             
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      //printf("\033[0;33m%d\n\033[0m", e[position]);
      //printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
token_end:
  //printf("token_end!\n");
  tokens[nr_token++].type = TK_END; 
  //printf("\n");

  return true;
}



__attribute__((used)) static uint32_t expr_calc();

word_t expr(char *e, bool *success) {
  //printf("\033[0;33menter expr!\033[0m\n");
  if (!make_token(e)) {
    printf("\033[0;33mmake_token failed!\033[0m\n");
    *success = false;
    return 0;
  }
  uint32_t ret = expr_calc();
  //printf("\033[0;33mresult: 0x%08x\033[0m\n", ret);
  *success = true;
  return ret;
  /* TODO: Insert codes to evaluate the expression. */
  // TODO();

  // return 0;
}

/*
static struct {
  int token_type;
  uint32_t level;
} token_level[] __attribute__((used)) = {
  { TK_END, 0},
  { ')', 1},
  { TK_EQ, 2},
  { '+', 3},
  { '-', 3},
  { '*', 4},
  { '/', 4},
  { TK_MEM, 8},
  { '(', 9},
};
*/
static uint32_t token_level(uint32_t token_type) {
  switch (token_type) {
    case TK_END: return 0;
    case ')': return 1;
    case TK_EQ: return 2;
    case '+': return 3;
    case '-': return 3;
    case '*': return 4;
    case '/': return 4;

    case TK_MEM: return 8;
    case '(': return 9;
    default: return 0xffffffff;
  }
}

__attribute__((used)) 
static Token pop(Stack * stack) {
    assert(stack->top >= 0);
    return stack->tokens[stack->top--];

}
__attribute__((used)) 
static void push(Stack * stack, Token token) {
    assert(stack->top < STACK_DEPTH);
    //printf("push token begin: %d\n", token.type);
    stack->tokens[++stack->top] = token;
    //printf("push token end: %d\n", stack->tokens[stack->top].type);
}
__attribute__((used)) 
static Token top(Stack * stack) {
    return stack->tokens[stack->top];
}






static void calc_symb_stack_top() {
  uint32_t a = 0;
  uint32_t b = 0;
  //printf("symb_stack.tokens[symb_stack.top].type: %d\n", symb_stack.tokens[symb_stack.top].type);
  switch (symb_stack.tokens[symb_stack.top].type) {
    case '+': 
      assert(num_stack.top > 0);
      b = num_stack.nums[num_stack.top--];
      a = num_stack.nums[num_stack.top--];
      assert(num_stack.top < 127);
      //printf("0x%08x + 0x%08x = 0x%08x\n", a, b, a+b);
      num_stack.nums[++num_stack.top] = a+b;
      pop(&symb_stack);
      return;
    case '-': 
      assert(num_stack.top > 0);
      b = num_stack.nums[num_stack.top--];
      a = num_stack.nums[num_stack.top--];
      assert(num_stack.top < 127);
      //printf("0x%08x - 0x%08x = 0x%08x\n", a, b, a-b);
      num_stack.nums[++num_stack.top] = a-b;
      pop(&symb_stack);
      return;
    case '*': 
      assert(num_stack.top > 0);
      b = num_stack.nums[num_stack.top--];
      a = num_stack.nums[num_stack.top--];
      assert(num_stack.top < 127);
      //printf("0x%08x * 0x%08x = 0x%08x\n", a, b, a*b);
      num_stack.nums[++num_stack.top] = a*b;
      pop(&symb_stack);
      return;
    case '/': 
      assert(num_stack.top > 0);
      b = num_stack.nums[num_stack.top--];
      a = num_stack.nums[num_stack.top--];
      assert(num_stack.top < 127);
      // div 0 error!
      assert(b!=0);
      //printf("0x%08x / 0x%08x = 0x%08x\n", a, b, a/b);
      num_stack.nums[++num_stack.top] = a/b;
      pop(&symb_stack);
      return;
    case '(': 
      return;
    case TK_MEM:
      assert(num_stack.top >= 0);
      uint32_t vaddr = num_stack.nums[num_stack.top--];
      assert(num_stack.top < 127);
      #include "memory/vaddr.h"
      uint32_t data = vaddr_read(vaddr, 4);
      num_stack.nums[++num_stack.top] = data;
      //printf("memory input addr: 0x%08x\n", vaddr);
      pop(&symb_stack);
      return;
    case TK_NEG:
      assert(num_stack.top >= 0);
      a = 0;
      b = num_stack.nums[num_stack.top--];
      assert(num_stack.top < 127);
      num_stack.nums[++num_stack.top] = 0 - b;
      pop(&symb_stack);
      return;


    default: 
      assert(0);
  }
}

static void case_token (Token token);

__attribute__((used)) 
static uint32_t expr_calc() {
  for (int i = 0; i < nr_token; i++) {
    switch (tokens[i].type) {
      case TK_NUM:
        char * end = tokens[i].str + strlen(tokens[i].str) + 1;
        num_stack.nums[++num_stack.top] = strtoul(tokens[i].str, &end, 0);
        //printf("push number: %d\n", num_stack.nums[num_stack.top]);
        break;
      case TK_REG:
        bool succ = false;
        word_t reg_val = isa_reg_str2val(tokens[i].str, &succ);
        assert(succ);
        num_stack.nums[++num_stack.top] = reg_val;
        //printf("push number from Register: 0x%08x\n", num_stack.nums[num_stack.top]);
        break;
      case '+':
      case '-':
      case '*':
      case '/':
        case_token(tokens[i]); break;

      // case '+':
        // case_token(tokens[i]); break;
        //  if (  (symb_stack.top < 0) || 
        //        (symb_stack.tokens[symb_stack.top].type == '(') ||
        //        ( token_level( symb_stack.tokens[symb_stack.top].type) < token_level('+') )                 
        //  ){
        //    printf("push symbol '+' to symb_stack!\n");
        //    push(&symb_stack, tokens[i]);
        //    bre )
        //  }
        //  while ( (symb_stack.tokens[symb_stack.top].type == '(') && (symb_stack.top >= 0) && token_level( symb_stack.tokens[symb_stack.top].type) >= token_level('+')) 
        //  { 
        //    //printf("loop ... , i = %d\n", i);
        //    calc_symb_stack_top();
        //  }
        //  //printf("loop end\n");
        //  push(&symb_stack, tokens[i]);
        //  break;
      // case '-':
        // case_token(tokens[i]); break;
        //  if (  (symb_stack.top < 0) || 
        //        (symb_stack.tokens[symb_stack.top].type == '(') ||
        //        ( token_level( symb_stack.tokens[symb_stack.top].type) < token_level('-') )                 
        //  ){
        //    printf("push symbol '-' to symb_stack!\n");
        //    push(&symb_stack, tokens[i]);
        //    break;
        //  }
        //  while ( (symb_stack.tokens[symb_stack.top].type == '(') && (symb_stack.top >= 0) && token_level( symb_stack.tokens[symb_stack.top].type) >= token_level('-')) 
        //  { 
        //    //printf("loop ... , i = %d\n", i);
        //    calc_symb_stack_top();
        //  }
        //  //printf("loop end\n");
        //  push(&symb_stack, tokens[i]);
        //  break;
      // case '*':
        // case_token(tokens[i]); break;
        //if (  (symb_stack.top < 0) || 
        //        (symb_stack.tokens[symb_stack.top].type == '(') ||
        //        ( token_level( symb_stack.tokens[symb_stack.top].type) < token_level('*') )                 
        //  ){
        //    printf("push symbol '*' to symb_stack!\n");
        //    push(&symb_stack, tokens[i]);
        //    break;
        //  }
        //  while ( (symb_stack.tokens[symb_stack.top].type == '(') && (symb_stack.top >= 0) && token_level( symb_stack.tokens[symb_stack.top].type) >= token_level('*')) 
        //  { 
        //    //printf("loop ... , i = %d\n", i);
        //    calc_symb_stack_top();
        //  }
        //  //printf("loop end\n");
        //  push(&symb_stack, tokens[i]);
        //  break;
      // case '/':
        // case_token(tokens[i]); break;
        //if (  (symb_stack.top < 0) || 
        //        (symb_stack.tokens[symb_stack.top].type == '(') ||
        //        ( token_level( symb_stack.tokens[symb_stack.top].type) < token_level('/') )                 
        //  ){
        //    printf("push symbol '/' to symb_stack!\n");
        //    push(&symb_stack, tokens[i]);
        //    break;
        //  }
        //  while ( (symb_stack.tokens[symb_stack.top].type == '(') && (symb_stack.top >= 0) && token_level( symb_stack.tokens[symb_stack.top].type) >= token_level('/')) 
        //  { 
        //    //printf("loop ... , i = %d\n", i);
        //    calc_symb_stack_top();
        //  }
        //  //printf("loop end\n");
        //  push(&symb_stack, tokens[i]);
        //  break;
      case TK_MEM:
        while (true) {
          if (  (symb_stack.top < 0) || 
                (symb_stack.tokens[symb_stack.top].type == '(') ||
                ( token_level( symb_stack.tokens[symb_stack.top].type) < token_level(TK_MEM) )
          ){
            push(&symb_stack, tokens[i]);
            break;
          }
          calc_symb_stack_top();
        }
        break;
      case '(':
        push(&symb_stack, tokens[i]);
        break;
      case ')':
          // if (  (symb_stack.top < 0) || 
          //       ( token_level( symb_stack.tokens[symb_stack.top].type) < token_level(')') )                 
          // ){
          //   //printf("push symbol '+' to symb_stack!\n");
          //   push(&symb_stack, tokens[i]);
          //   break;
          // }
          while ( (symb_stack.top >= 0) && token_level( symb_stack.tokens[symb_stack.top].type) >= token_level(')')) 
          { 
            if (symb_stack.tokens[symb_stack.top].type == '(') {
              //printf("pop `)` & `(`-40-: %d\n", symb_stack.tokens[symb_stack.top].type);
              pop(&symb_stack);
              break;
            }
            //printf("loop ... , i = %d\n", i);
            calc_symb_stack_top();
          }
          //printf("loop end\n");
          //push(&symb_stack, tokens[i]);
          break;

      case TK_END:
        //printf("The last calc begin!\n");
        while ( (num_stack.top) || (symb_stack.top >= 0) ) calc_symb_stack_top();
        //printf("The last calc end!\n");
        //printf("\tResult: 0x%08x\n", num_stack.nums[num_stack.top]);
      default:
        break;
    }
  }

  //printf("num stack top: %d, symbol stack top: %d\n", num_stack.top, symb_stack.top);
  assert(num_stack.top == 0);
  assert(symb_stack.top == -1);
  return num_stack.nums[num_stack.top];
}

static void case_token (Token token) {
  if (  (symb_stack.top < 0) || 
        (symb_stack.tokens[symb_stack.top].type == '(' ) ||
        ( token_level( symb_stack.tokens[symb_stack.top].type) < token_level(token.type) )              
  ){
    push(&symb_stack, token);
    return;
  }
  while ( (symb_stack.tokens[symb_stack.top].type != '(') && (symb_stack.top >= 0) && token_level( symb_stack.tokens[symb_stack.top].type) >= token_level(token.type)) 
  { 
    calc_symb_stack_top();
  }
  push(&symb_stack, token);
}
