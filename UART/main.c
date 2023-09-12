#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "uart_hal.h"

int main(void)
{
    uart_init(9600, 0); // Initiate UART
	uint8_t data;
	
	sei(); // Enable global interrupts
	
	uart_send_string("Communication Start\n\r"); // Send string "Communication Start", with a new line inserted after, and cursor at the start of the line
	
    while (1) 
    {
		if (uart_read_count() > 0) // If there are unread messages, read them
		{
			data = uart_read_byte(); // Read the message into data variable
			uart_send_byte(data); // Echo the same data back
		}
    }
}

