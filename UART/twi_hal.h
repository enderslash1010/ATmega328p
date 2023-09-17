
#ifndef TWI_HAL_H_
#define TWI_HAL_H_

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

// General Status Codes
#define TWI_START         0x08
#define TWI_RE_START      0x10
#define TWI_ERROR         0x38
#define TWI_NONE          0xF8

// Master Transmitter (MT) Mode Status Codes
#define TWI_MT_SLA_W_ACK  0x18
#define TWI_MT_SLA_W_NACK 0x20
#define TWI_MT_DATA_ACK   0x28
#define TWI_MT_DATA_NACK  0x30

// Master Receiver (MR) Mode Status Codes
#define TWI_MR_SLA_R_ACK  0x40
#define TWI_MR_SLA_R_NACK 0x48
#define TWI_MR_DATA_ACK   0x50
#define TWI_MR_DATA_NACK  0x58

// Return values
enum
{
	TWI_OK,
	TWI_ERROR_START,
	TWI_ERROR_RE_START,
	TWI_ERROR_SLA_R,
	TWI_ERROR_SLA_W,
	TWI_ERROR_DATA_R,
	TWI_ERROR_DATA_W,
	TWI_NACK
};

// Timeout value
#define TWI_TIMEOUT 1600

void twi_init(uint32_t speed);
uint8_t twi_write(uint8_t addr, uint8_t data);
uint8_t twi_write_bytes(uint8_t addr, uint8_t *data, uint16_t len);

#endif /* TWI_HAL_H_ */