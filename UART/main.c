#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <string.h>
#include "uart_hal.h"
#include "LCD.h"
#include "calculator.h"
#include "util.h"

#define EXPRESSION_LENGTH 50

int main(void)
{
	sei(); // Enable global interrupts
	
    uart_init(9600, 0); // Initiate UART communication
	uint8_t data; // variable to load byte from UART communication
	
	LCD_t lcd = LCD_init(0b111, 2, 16, 100000, 1); // Initialize LCD and TWI communication
	LCD_display_toggle(&lcd, 1, 1, 1); // Turn cursor and cursor blink on
	
	uart_send_string("\n\rCommunication Start:\n\r"); // Send string "Communication Start", with a new line inserted after, and cursor at the start of the line
	
	char expression[EXPRESSION_LENGTH] = ""; // Initialize expression to empty string
	int length = 0; // Initialize length of expression to 0
	
	uint8_t isShowingResult = 0; // state variable; true after pressing enter/showing result of expression, used to know when to clear the LCD screen
	
    while (1) 
    {
		if (uart_read_count() > 0) // If there are unread messages, read them
		{
			data = uart_read_byte(); // Read the message into data variable
			
			if (data == 0xD) // Enter key is pressed
			{
				if (length == 0) // If expression is empty
				{
					uart_send_string("Please input an expression.\n\r");
					LCD_clear_display(&lcd);
					LCD_display_toggle(&lcd, 1, 0, 0); // Hide cursor
					LCD_write_string(&lcd, "Please input");
					LCD_set_cursor(&lcd, 1, 0);
					LCD_write_string(&lcd, "an expression.");
					isShowingResult = 1;
				}
				else // Expression is non-empty
				{
					int64_t val = infixEval(expression, length); // evaluate the expression
					char buffer[20]; // Declare buffer for result of expression
					int64ToStringBuffer(buffer, val); // convert integer val into a string in buffer
					
					// Send to host pc via UART
					uart_send_string(" = "); // send equals sign
					uart_send_string(buffer); // send the contents of buffer
					uart_send_string("\n\r"); // go to beginning of newline
					
					// Put result on LCD, return cursor to home
					LCD_display_toggle(&lcd, 1, 0, 0); // Hide cursor
					LCD_set_cursor(&lcd, 1, 0);
					LCD_write_data(&lcd, '=');
					LCD_write_string(&lcd, buffer);
					LCD_return_home(&lcd);
					isShowingResult = 1; // LCD is now showing result
					
					// Clear expression from memory, and set length to 0
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
					
					// Put data on LCD
					if (isShowingResult) 
					{
						LCD_clear_display(&lcd);
						LCD_display_toggle(&lcd, 1, 1, 1); // Show cursor and blink
						isShowingResult = 0;
					}
					if (lcd.currCol >= 15) LCD_cursor_display_shift(&lcd, 1, 0); // shift display if at right edge of screen
					LCD_write_data(&lcd, data); // write character to LCD
				}
			}
		}
    }
}
