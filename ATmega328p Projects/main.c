#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define delay 100 // The delay, in milliseconds, put between LED operations (i.e. the time duration one LED should be ON before it's turned OFF)
#define numModes 2 // The number of modes for the LEDs to cycle through

void colorDot();
void colorTrail();
void colorBlink(int num);

// Used for the number of blinks when pressing the button
// Needs to be volatile since this value is modified in an interrupt (the compiler shouldn't try to predict blinkCount by this assignment statement)
volatile unsigned int blinkCount = 1;

// Handle interrupt
ISR(INT1_vect) 
{
	colorBlink(blinkCount++);
}

int main(void)
{
	
	DDRB = 0b00000111; // Set all LED ports using port B to output
	DDRD = 0b11110100; // Set all LED ports using port D to output, interrupt INT1 to input, PD0 and PD1 are reserved for communication with host
	
	PORTD = (1 << PORTD3); // Set port D3 to high, and since DDD3/INT1 is set to input, the pull-up resistor is activated
	
	EICRA = (1 << ISC11) | (0 << ISC10); // Set ISC11 to 1 and ISC10 to 0, this generates an interrupt request on the falling edge of INT1
	EIMSK = (1 << INT1); // Set INT1 to 1 in interrupt mask register (Enables interrupts on INT1)
	
	sei(); // Global interrupt enable
	
	// counter used to switch between different modes (i.e. colorDot and colorTrial)
	// an unsigned int allows counter to go back to 0 when it overflows (to avoid undefined behavior with signed int overflow)
	unsigned int counter = 0;
	
	while (1)
	{
		switch (counter % numModes) // Select what the LEDs do based on the counter
		{
			case 0:
				colorDot();
				break;
			case 1:
				colorTrail();
				break;
		}
		counter++; // Increment counter to go to next mode
	}
}

// sets the LEDNum light to state (zero is OFF, non-zero is ON)
inline void set(unsigned int LEDNum, int state) 
{
	switch (LEDNum) 
	{
		case 0:
			if (state) PORTB |= (1<<PORTB0);
			else PORTB &= ~(1<<PORTB0);
			break;
		case 1:
			if (state) PORTB |= (1<<PORTB1);
			else PORTB &= ~(1<<PORTB1);
			break;
		case 2:
			if (state) PORTD |= (1<<PORTD2);
			else PORTD &= ~(1<<PORTD2);
			break;
		case 3:
			if (state) PORTB |= (1<<PORTB2);
			else PORTB &= ~(1<<PORTB2);
			break;
		case 4:
			if (state) PORTD |= (1<<PORTD4);
			else PORTD &= ~(1<<PORTD4);
			break;
		case 5:
			if (state) PORTD |= (1<<PORTD5);
			else PORTD &= ~(1<<PORTD5);
			break;
		case 6:
			if (state) PORTD |= (1<<PORTD6);
			else PORTD &= ~(1<<PORTD6);
			break;
		case 7:
			if (state) PORTD |= (1<<PORTD7);
			else PORTD &= ~(1<<PORTD7);
			break;
	}
}

// LEDs are toggled in a line, only one LED is ON at a time
void colorDot() 
{
	for (int i = 0; i < 8; i++) 
	{
		set(i, 1); // Turn LED ON
		_delay_ms(delay);
		set(i, 0); // Turn LED OFF
	}
}

// LEDs are toggled in a line, and stay ON until all LEDs are ON, and then all LEDs are turned off in a line
void colorTrail() 
{
	for (int i = 0; i < 8; i++) 
	{
		set(i, 1);
		_delay_ms(delay);
	}
	
	for (int i = 0; i < 8; i++) 
	{
		set(i, 0);
		_delay_ms(delay);
	}
}

// LEDs are blinked ON/OFF n number of times
void colorBlink(int n) 
{
	for (int i = 0; i < n; i++) 
	{
		// Turn all LEDs ON
		for (int i = 0; i < 8; i++) set(i, 1);
		_delay_ms(500);
		
		// Turn all LEDs OFF
		for (int i = 0; i < 8; i++) set(i, 0);
		_delay_ms(500);
	}
}

