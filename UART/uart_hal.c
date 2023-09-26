
#include "uart_hal.h"

volatile static uint8_t rx_buffer[RX_BUFFER_SIZE] = {0}; // Circular buffer
volatile static uint16_t rx_count = 0;

ISR(USART_RX_vect) // Receiver Complete Interrupt
{
	volatile static int rx_write_pos = 0; // The index of rx_buffer to write a new byte to, and make this variable local to this function (static)
	
	rx_buffer[rx_write_pos++] = UDR0; // Get the data in the RXB in UDR0 (reading from UDR0 returns the contents of RXB), and increment rx_write_pos after
	rx_count++; // Increase the number of received messages by 1
	rx_write_pos %= RX_BUFFER_SIZE; // Limit rx_write_pos to be less than RX_BUFFER_SIZE, so rx_write_pos == RX_BUFFER_SIZE wraps around to 0
}

// Initializes registers for UART communication with host computer
void uart_init(uint32_t baudRate, uint8_t high_speed) 
{
	uint8_t speed = 16; // Initialize speed to 16
	if (high_speed)
	{
		UCSR0A |= (1 << U2X0); // enable double speed
		speed = 8; // Change speed value
	}
	
	uint32_t UBRR0Val = (F_CPU/(speed*baudRate)) - 1; // Calculate UBRR0 using formula from datasheet
	
	// Configure Baud Rate Registers
	UBRR0H = (UBRR0Val & 0x0F00) >> 8;
	UBRR0L = (UBRR0Val & 0x00FF);
	
	// Configure USART Control and Status Register 0B
	UCSR0B |= (1 << RXCIE0) // Receiver Interrupt Enable 0 (We don't need to do anything on transmitter complete)
			| (1 << RXEN0) | (1 << TXEN0); // Receiver/Transmitter Enable 0
			
	// Set parity mode
	UCSR0C |= (1 << UPM01); // parity mode enabled, even parity
}

// Sends one byte through the transmitter
void uart_send_byte(uint8_t c) 
{
	while (!(UCSR0A & (1 << UDRE0))); // Wait until the transmitter buffer is empty
	UDR0 = c; // Put data in TXB buffer in UDR0 Register (The TXB buffer is the destination for data written to a UDRn register)
}

// Sends an array of bytes through the transmitter
void uart_send_array(uint8_t *c, int length)
{
	for (int i = 0; i < length; i++) {
		uart_send_byte(c[i]); // Send length number of bytes from array c through transmitter
	}
}

// Send a string through the transmitter
void uart_send_string(char *str)
{
	int index = 0;
	while (str[index] != '\0') {
		uart_send_byte(str[index]); // Sends each char of a string through the transmitter
		index++; // Increment to next char
	}
	uart_send_byte('\0'); // End string by sending null terminator byte
}

// Returns the number of messages waiting to be read
uint16_t uart_read_count(void)
{
	return rx_count;
}

// Returns the next data byte from the rx_buffer
uint8_t uart_read_byte(void)
{
	static uint16_t rx_read_pos = 0; // The index of rx_buffer to read from, and make this variable local to this function (static)
	uint8_t data = 0; // Initialize data to 0
	
	data = rx_buffer[rx_read_pos++]; // Set data to the byte of rx_buffer at rx_read_pos, increment rx_read_pos by 1 after
	rx_count--; // Decrement rx_count by 1, because a message has been received and read
	rx_read_pos %= RX_BUFFER_SIZE; // Wrap rx_read_pos back to 0 when it reaches RX_BUFFER_SIZE
	
	return data;
}
