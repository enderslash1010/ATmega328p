
#ifndef CALCULATOR_H_
#define CALCULATOR_H_

#include <stdint.h>

char operatorStack[100];
char* operatorSP;

int64_t operandStack[100];
int64_t* operandSP;

#define push(sp, n) (*((sp)++) = (n))
#define pop(sp) (*--(sp))
#define peek(sp) (*(sp - 1))

int64_t infixEval(char *infix, int length);
int getOperatorPrecedence(char op);

#endif /* CALCULATOR_H_ */
