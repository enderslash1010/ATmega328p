#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <string.h>
#include "uart_hal.h"
#include "calculator.h"
#include <stdlib.h>

#define EXPRESSION_LENGTH 50

int main(void)
{
    uart_init(9600, 0); // Initiate UART communication
	uint8_t data; // variable to load byte from UART communication
	
	sei(); // Enable global interrupts
	
	uart_send_string("Communication Start\n\r"); // Send string "Communication Start", with a new line inserted after, and cursor at the start of the line
	
	char expression[EXPRESSION_LENGTH] = ""; // Initialize expression to empty string
	int length = 0; // Initialize length of expression to 0
	
    while (1) 
    {
		if (uart_read_count() > 0) // If there are unread messages, read them
		{
			data = uart_read_byte(); // Read the message into data variable
			
			if (data == 0xD) // Enter key is pressed
			{
				if (length == 0) // If expression is empty
				{
					uart_send_string("Please input an expression.");
				}
				else // Expression is non-empty
				{
					int val = infixEval(expression, length); // evaluate the expression
					char buffer[EXPRESSION_LENGTH + 1]; // Extra element in buffer, to ensure there is a null terminator
					itoa(val, buffer, 10); // convert integer val into a string in buffer
					
					uart_send_string(" = "); // send equals sign
					uart_send_string(buffer); // send the contents of buffer
					uart_send_string("\n\r"); // go to beginning of newline
					
					// Clear expression, and set length to 0
					memset(expression, '\0', sizeof(expression));
					length = 0;
				}
			}
			else // Other key is pressed
			{
				// and data contains an accepted character
				if (length != EXPRESSION_LENGTH && // Ensure the expression string is not full, and
					((data >= 0x30 && data <= 0x39) || // Ensure data is a number, or...
					(data == 0x2B) || (data == 0x2D) || (data == 0x2A) || (data == 0x2F) || // a operator (+,-,*,/), or...
					(data == 0x20) || // a space, or...
					(data == 0x28) || (data == 0x29))) // parentheses
				{
					expression[length++] = (char) data; // Add data to expression, and increment length after
					uart_send_byte(data); // Echo data on host computer
				}
			}
		}
    }
}
