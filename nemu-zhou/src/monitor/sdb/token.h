#ifndef __TOKEN_STACK_H__
#define __TOKEN_STACK_H__
#include <stdint.h>
#include <assert.h>

#define STACK_DEPTH 127

typedef struct token {
  uint32_t type;
  char str[24];
} Token;

typedef struct __attribute__((aligned(4096))) {
    Token tokens[STACK_DEPTH];
    uint32_t top;
} Stack;

Token pop(Stack stack) {
    assert(stack.top > 0);
    return stack.tokens[stack.top--];

}
void push(Stack stack, Token token) {
    assert(stack.top < STACK_DEPTH);
    stack.tokens[++top] = token;
}
Token top(Stack stack) {
    return stack.tokens[stack.top];
}

bool parser() {

}


#endif // ! __TOKEN_STACK_H__