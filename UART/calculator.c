#include "calculator.h"

// Gets operator precedence of a specified operator
int getOperatorPrecedence(char op)
{
	switch (op) {
		case '*': case '/':
			return 2;
		case '+': case '-':
			return 1;
		default:
			return 0;
	}
}

void compute() {
	char operator = pop(operatorSP); // pop operator from operator stack
	
	// pop operand stack twice
	int operand2 = pop(operandSP);
	int operand1 = pop(operandSP);
	
	switch (operator) { // apply operator to operands, and push result to operand stack
		case '+':
			push(operandSP, operand1 + operand2);
			break;
		case '-':
			push(operandSP, operand1 - operand2);
			break;
		case '*':
			push(operandSP, operand1 * operand2);
			break;
		case '/':
			push(operandSP, operand1 / operand2);
			break;
	}
}

// TODO: Evaluates infix expression
int infixEval(char *infix, int length)
{
	// Initialize stacks
	operatorSP = operatorStack;
	operandSP = operandStack;
	
	for (int i = 0; i < length; i++)
	{
		char token = infix[i]; // Get the next token
		switch (token)
		{
			case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': // Token is a number
			{
				int num = token - '0';
				while (i+1 < length && (infix[i+1] >= 0x30 && infix[i+1] <= 0x39)) {
					num *= 10; // shift current decimal number one place to the left
					num += infix[i+1] - '0'; // put next number in 0's place
					i++;
				}
				push(operandSP, num); // push to operand stack
			}
				break;
			case '(':
				push(operatorSP, token); // push to operator stack
				break;
			case ')':
				while ((operatorSP - operatorStack > 0) && peek(operatorSP) != '(') { // while the operator on top of the operator stack is not '('
					compute();
				}
				pop(operatorSP); // pop '(' from operator stack and discard it
				break;
			case '+': case '-': case '*': case '/':
				while ((operatorSP - operatorStack > 0) && (getOperatorPrecedence(peek(operatorSP)) >= getOperatorPrecedence(token))) { // while the operator stack isn't empty and the operator on top of the stack has equal/greater precedence to the current operator
					compute();
				}
				push(operatorSP, token); // push the current operator
				break;
		}
	}

	while (operatorSP - operatorStack > 0) compute(); // while the operator stack isn't empty, compute	
		
	return pop(operandSP); // return the value at top of operand stack 
}

int infixEvalT(char *infix)
{
	operatorSP = operatorStack;
	operandSP = operandStack;
	
	if (infix[0] == 'e') return 1;
	return 0;
}
