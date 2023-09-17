#include "twi_hal.h"
#include "uart_hal.h"

volatile uint8_t status = 0xF8;

ISR(TWI_vect)
{
	status = (TWSR & 0xF8); // mask the prescaler and reserved bit
}

// Initiate TWI (I2C) communication with speed kHz bus
void twi_init(uint32_t speed)
{
	uint32_t gen_t = ((F_CPU/speed) - 16)/2; // Bit Rate Generator Unit (See ATmega328p datasheet pg 180)
	TWBR = gen_t & 0xFF; // TWBR is 2 bytes
	
	TWCR = (1 << TWEN) | (1 << TWIE); // Set TWI Enable Bit to 1, set TWI Interrupt Enable to 1
	
	// Prescaler Value is 1, so TWPS1 and TWPS0 are 0, so no need to set TWI Status Register (TWSR) since the initial values for TWPS1 and TWPS0 are 0
	
	// Enable pull-up resistors on SDA and SCL pins
	PORTC |= (1 << PORTC4) | (1 << PORTC5);
}

// Master sends start condition
static uint8_t twi_start(void)
{
	uint16_t timeoutTimer = 0;
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN) | (1 << TWIE); 
	
	// Wait for status to be TWI_START; if it doesn't change in time, then TWI_ERROR_START is returned
	while (status != TWI_START) if (++timeoutTimer >= TWI_TIMEOUT) return TWI_ERROR_START;
	return TWI_OK;
}

// Master sends stop condition
static void twi_stop(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN) | (1 << TWIE);
}

// Master sends repeated start condition
static uint8_t twi_re_start(void)
{
	uint16_t timeoutTimer = 0;
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN) | (1 << TWIE);
	
	// Wait for status to be TWI_RE_START; if it doesn't change in time, then TWI_ERROR_RE_START is returned
	while (status != TWI_RE_START) if (++timeoutTimer >= TWI_TIMEOUT) return TWI_ERROR_RE_START;
	return TWI_OK;
}

// Clears TWINT (by setting it to 1), and waits for SLA+R acknowledgment from slave
static uint8_t twi_sla_r(void)
{
	uint16_t timeoutTimer = 0;
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
	
	while (status != TWI_MR_SLA_R_ACK) if (++timeoutTimer >= TWI_TIMEOUT) return TWI_ERROR_SLA_R;
	return TWI_OK;
}

// Sends a data acknowledge or not acknowledge from master
static uint8_t twi_data_r_ack(uint8_t ack)
{
	uint16_t timeoutTimer = 0;
	if (ack != 0)
	{
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA); // Acknowledge
		while (status != TWI_MR_DATA_ACK) if (++timeoutTimer >= TWI_TIMEOUT) return TWI_ERROR_DATA_R;
	}
	else
	{
		TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE); // Not Acknowledge
		while (status != TWI_MR_DATA_NACK) if (++timeoutTimer >= TWI_TIMEOUT) return TWI_ERROR_DATA_R;
	}
	return TWI_OK;
}

// Clears TWINT (by setting it to 1), and waits for SLA+W acknowledgment from slave
static uint8_t twi_sla_w(void)
{
	uint16_t timeoutTimer = 0;
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
	
	while (status != TWI_MT_SLA_W_ACK) if (++timeoutTimer >= TWI_TIMEOUT) return TWI_ERROR_SLA_W;
	return TWI_OK;
}

// Clears TWINT (by setting it to 1), and waits for data acknowledgment from slave
static uint8_t twi_data_w_ack(void)
{
	uint16_t timeoutTimer = 0;
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
	
	while (status != TWI_MT_DATA_ACK) if (++timeoutTimer >= TWI_TIMEOUT) return TWI_ERROR_DATA_W;
	return TWI_OK;
}

// Writes data of length len in bytes to device address addr
uint8_t twi_write_bytes(uint8_t addr, uint8_t *data, uint16_t len)
{
	uint8_t err = TWI_OK;
	
	err = twi_start(); // Send START condition
	if (err != TWI_OK) // Validate START condition sent successfully
	{
		twi_stop(); // Send STOP condition if it's not OK
		return err;
	}
	
	TWDR = (addr << 1) | 0; // Enter MR Mode by transmitting SLA+W (By writing SLA+W to TWDR)
	
	err = twi_sla_w(); // Continue transfer by writing 1 to TWINT, and wait for acknowledged
	if (err != TWI_OK) // Validate SLA+W sent successfully, and received acknowledged
	{
		twi_stop(); // Send STOP condition if it's not OK
		return err;
	}
	
	// Transmit data packet (by writing the data byte to TWDR)
	for (int i = 0; i < len; i++)
	{
		TWDR = data[i]; // Write current data byte to transmit to TWDR
		err = twi_data_w_ack(); // Continue the transfer by writing 1 to TWINT, and wait for acknowledged
		if (err != TWI_OK) // Validate data sent successfully, and received acknowledged
		{
			twi_stop(); // Send STOP condition if it's not OK
			return err;
		}
	}
	
	twi_stop(); // Send STOP condition after all data is sent and acknowledged
	return err;
}

// Writes a data byte to device address addr
uint8_t twi_write(uint8_t addr, uint8_t data)
{
		uint8_t err = TWI_OK;
		
		err = twi_start(); // Send START condition
		if (err != TWI_OK) // Validate START condition sent successfully
		{
			twi_stop(); // Send STOP condition if it's not OK
			return err;
		}
		
		TWDR = (addr << 1) | 0; // Enter MR Mode by transmitting SLA+W (By writing SLA+W to TWDR)
		
		err = twi_sla_w(); // Continue transfer by writing 1 to TWINT, and wait for acknowledged
		if (err != TWI_OK) // Validate SLA+W sent successfully, and received acknowledged
		{
			twi_stop(); // Send STOP condition if it's not OK
			return err;
		}
		
		// Transmit data packet (by writing the data byte to TWDR)
		TWDR = data; // Write current data byte to transmit to TWDR
		err = twi_data_w_ack(); // Continue the transfer by writing 1 to TWINT, and wait for acknowledged
		if (err != TWI_OK) // Validate data sent successfully, and received acknowledged
		{
			twi_stop(); // Send STOP condition if it's not OK
			return err;
		}
		
		// Code for debugging data sent via TWI
		/*
		uart_send_string("Data sent: ");
		char buffer[20];
		itoa(data, buffer, 2);
		uart_send_string("0b");
		uart_send_string(buffer);
		uart_send_string("\n\r");
		*/
		
		twi_stop(); // Send STOP condition after all data is sent and acknowledged
		return err;
}
