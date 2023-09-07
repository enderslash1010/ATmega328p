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
ISR(INT1_vect) {
	cli(); // Disable interrupts when handling an interrupt (a button press)
	colorBlink(blinkCount++);
	sei(); // Enable global interrupts when done
}

int main(void)
{
	
	DDRB |= 0b00000111; // Set all LED ports using port B to output
	DDRD |= 0b11110100; // Set all LED ports using port D to output, interrupt INT1 to input, PD0 and PD1 are reserved for communication with host
	
	PORTD |= (1 << PORTD3); // Set port D3 to high, and since DDD3/INT1 is set to input, the pull-up resistor is activated
	
	EICRA |= (0b10 << ISC10); // Set ISC11 to 1 and ISC10 to 0, this generates an interrupt request on the falling edge of INT1
	EIMSK = (1 << INT1); // Set INT1 to 1 in interrupt mask register (Enables interrupts on INT1)
	
	sei(); // Global interrupt enable
	
	// counter used to switch between different modes (i.e. colorDot and colorTrial)
	// an unsigned int allows counter to go back to 0 when it overflows (to avoid undefined behavior with signed int overflow)
	unsigned int counter = 0;
	
	while (1)
	{
		switch (counter % numModes) { // Select what the LEDs do based on the counter
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

// LEDs are toggled in a line, only one LED is ON at a time
void colorDot() {
	for (int i = 0; i < 2; i++) {
		PORTB |= (1<<i);
		_delay_ms(delay);
		PORTB &= ~(1<<i);
	}
	
	PORTD |= (1<<PORTD2);
	_delay_ms(delay);
	PORTD &= ~(1<<PORTD2);
	
	PORTB |= (1<<PORTB2);
	_delay_ms(delay);
	PORTB &= ~(1<<PORTB2);
	
	for (int i = 4; i < 8; i++) {
		PORTD |= (1<<i);
		_delay_ms(delay);
		PORTD &= ~(1<<i);
	}
}

// LEDs are toggled in a line, and stay ON until all LEDs are ON, and then all LEDs are turned off in a line
void colorTrail() {
	// Turn all colors on individually
	for (int i = 0; i < 2; i++) {
		PORTB |= (1<<i);
		_delay_ms(delay);
	}
	
	PORTD |= (1<<PORTD2);
	_delay_ms(delay);
	
	PORTB |= (1<<PORTB2);
	_delay_ms(delay);
	
	for (int i = 4; i < 8; i++) {
		PORTD |= (1<<i);
		_delay_ms(delay);
	}
	
	// Turn all colors off individually
	for (int i = 0; i < 2; i++) {
		PORTB &= ~(1<<i);
		_delay_ms(delay);
	}
	
	PORTD &= ~(1<<PORTD2);
	_delay_ms(delay);
	
	PORTB &= ~(1<<PORTB2);
	_delay_ms(delay);
	
	for (int i = 4; i < 8; i++) {
		PORTD &= ~(1<<i);
		_delay_ms(delay);
	}
}

// LEDs are blinked ON/OFF n number of times
void colorBlink(int n) {
	for (int i = 0; i < n; i++) {
		// Turn all on
		for (int j = 0; j < 8; j++) {
			switch (j) {
				case 0: case 1:
				PORTB |= (1 << j);
				break;
				case 3:
				PORTB |= (1 << PORTB2);
				break;
				case 2: case 4: case 5: case 6: case 7:
				PORTD |= (1 << j);
				break;
			}
		}
		_delay_ms(500);
		// Turn all off
		for (int j = 0; j < 8; j++) {
			switch (j) {
				case 0: case 1:
				PORTB &= ~(1 << j);
				break;
				case 3:
				PORTB &= ~(1 << PORTB2);
				break;
				case 2: case 4: case 5: case 6: case 7:
				PORTD &= ~(1 << j);
				break;
			}
		}
		_delay_ms(500);
	}
}

