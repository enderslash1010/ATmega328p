
#ifndef UART_HAL_H_
#define UART_HAL_H_

#define F_CPU 16000000UL
#define RX_BUFFER_SIZE 128
#define READ_WRITE_BUFFER_START 0

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdlib.h>
#include "uart_hal.h"

void uart_init(uint32_t baudRate, uint8_t high_speed);
void uart_send_byte(uint8_t c);
void uart_send_array(uint8_t *c, int length);
void uart_send_string(char *str);
uint16_t uart_read_count(void);
uint8_t uart_read_byte(void);

#endif /* UART_HAL_H_ */
