
#ifndef CALCULATOR_H_
#define CALCULATOR_H_

#include <stdlib.h>

char operatorStack[100];
char* operatorSP;

int operandStack[100];
int* operandSP;

#define push(sp, n) (*((sp)++) = (n))
#define pop(sp) (*--(sp))
#define peek(sp) (*(sp - 1))

int infixEval(char *infix, int length);
int getOperatorPrecedence(char op);

#endif /* CALCULATOR_H_ */
