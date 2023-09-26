
#include "LCD.h"
#include "twi_hal.h"
#include <util/delay.h>

// Sends data via TWI
static uint8_t send_twi(LCD_t *lcd, uint8_t data)
{
	return twi_write(lcd->addr, data | (lcd->isBacklightOn << BACKLIGHT));
}

// Sends Enable pulse
static uint8_t send_enable(LCD_t *lcd, uint8_t data)
{
	uint8_t err = send_twi(lcd, data | (1 << E)); // Send Enable high
	_delay_us(1); // Enable pulse needs to be high for at least 230ns
	if (err != TWI_OK) return err;
	
	err = send_twi(lcd, data & ~(1 << E)); // Send Enable low
	_delay_us(50); // Commands need a minimum of 37us to settle
	return err;
}

// Initializes LCD and runs through HD44780U Initialization process for 4 bit mode
// Intializes cursor ON and cursor blink ON
LCD_t LCD_init(uint8_t addr, uint8_t rows, uint8_t cols, uint32_t bus_speed, uint8_t isPCF8574A)
{
	// Initiate LCD struct
	LCD_t lcd;
	lcd.addr = addr & 0b111; // hardware selectable address is 3 bits
	lcd.rows = rows;
	lcd.cols = cols;
	lcd.isBacklightOn = 1;
	lcd.currRow = 0;
	lcd.currCol = 0;
	
	// Initialize fixed address
	if (isPCF8574A) lcd.addr |= (PCF8574A_FIXED_ADDR << 3);
	else lcd.addr |= (PCF8574_FIXED_ADDR << 3);
	
	// Initiate TWI(I2C) communication
	twi_init(bus_speed);
	
	// Go through initialization instructions for LCD (from data sheet)
	_delay_ms(20); // Wait more than 15ms after Vcc rises to 4.5V
	send_twi(&lcd, 0); // Pull RS and R/W low to begin sending commands
	send_enable(&lcd, 0b0011 << 4); // Special function set
	_delay_ms(5); // Wait for more than 4.1ms
	send_enable(&lcd, 0b0011 << 4); // Special function set
	_delay_us(200); // Wait for more than 100us
	send_enable(&lcd, 0b0011 << 4); // Special function set
	_delay_us(200); // Wait for more than 100us
	send_enable(&lcd, 0b0010 << 4); // Initial function set to 4 bit
	_delay_us(200); // Wait for more than 100us
	// Now in 4-bit mode, can send 'regular' 4 bit commands
	LCD_function_set(&lcd, 0, 1, 0); // Regular function set
	LCD_display_toggle(&lcd, 0, 0, 0); // Display OFF (D=0, C=0, B=0)
	LCD_clear_display(&lcd); // Clear display
	LCD_entry_mode_set(&lcd, 1, 0); // Entry mode set (I/D=1 , S=0)
	// End of Initialization
	
	// Display ON
	LCD_display_toggle(&lcd, 1, 1, 0);
	
	return lcd;
}

// Sends a byte via TWI
static uint8_t send_byte(LCD_t *lcd, uint8_t data, uint8_t mode)
{
	mode &= 0xF; // ignore most significant byte of mode
	
	// Determine MS Nibble and LS Nibble
	uint8_t MSNibble = (data & 0xF0) | mode;
	uint8_t LSNibble = ((data << 4) & 0xF0) | mode;
	
	uint8_t err = send_enable(lcd, MSNibble); // Enable pulse
	if (err != TWI_OK) return err;
	
	err = send_enable(lcd, LSNibble); // Enable pulse
	return err;
}

// Clears all display data, returns cursor to original status (brings cursor to left edge on first line of display, text goes from left->right)
uint8_t LCD_clear_display(LCD_t *lcd)
{
	uint8_t err = send_byte(lcd, 0b00000001, 0);
	lcd->currRow = 0;
	lcd->currCol = 0;
	_delay_ms(3);
	return err;
}

// Returns cursor to home, and display to original status, if shifted
uint8_t LCD_return_home(LCD_t *lcd)
{
	uint8_t err = send_byte(lcd, 0b00000010, 0);
	lcd->currRow = 0;
	lcd->currCol = 0;
	_delay_ms(3);
	return err;
}

// Sets the moving direction of the cursor and display
// isLtR: 1 -> cursor moves left to right; 0 -> cursor moves right to left
// isShift: shifts display according to the direction (from isLtR)
uint8_t LCD_entry_mode_set(LCD_t *lcd, uint8_t isLtR, uint8_t isShift)
{
	uint8_t code = 0b00000100 | ((isLtR & 0x1) << 1) | ((isShift & 0x1) << 0);
	uint8_t err = send_byte(lcd, code, 0);
	_delay_us(50);
	return err;
}

// Controls display/cursor/blink state
// isDisplayOn: 1 -> entire display is turned on; 0 -> display is turned off, but display data remains in DDRAM
// isCursorOn: 1 -> cursor on; 0 -> cursor off
// isCursorBlink: 1 -> cursor blink on; 0 -> cursor blink off
uint8_t LCD_display_toggle(LCD_t *lcd, uint8_t isDisplayOn, uint8_t isCursorOn, uint8_t isCursorBlink)
{
	uint8_t code = 0b00001000 | ((isDisplayOn & 0x1) << 2) | ((isCursorOn & 0x1) << 1) | ((isCursorBlink & 0x1) << 0);
	uint8_t err = send_byte(lcd, code, 0);
	_delay_us(50);
	return err;
}

// Moves cursor and shifts display without changing DDRAM
// isDisplayShift: 1 -> shifts entire display; 0 -> only shifts cursor position
// isLtR: 1 -> shifts to right; 0 -> shifts to left
uint8_t LCD_cursor_display_shift(LCD_t *lcd, uint8_t isDisplayShift, uint8_t isLtR)
{
	uint8_t code = 0b00010000 | ((isDisplayShift & 0x1) << 3) | ((isLtR & 0x1) << 2);
	uint8_t err = send_byte(lcd, code, 0);
	_delay_us(50);
	return err;
}

// Function set instruction
// dataLength: 1 -> 8-bit; 0 -> 4-bit
// numLines: 1 -> 2 line display; 0 -> 1 line display
// font: 1 -> 5x10 dot character font; 0 -> 5x10 dot character font (NOTE: two line displays ignore this, and always use 5x10)
uint8_t LCD_function_set(LCD_t *lcd, uint8_t dataLength, uint8_t numLines, uint8_t font)
{
	uint8_t code = 0b00100000 | ((dataLength & 0x1) << 4) | ((numLines & 0x1) << 3) | ((font & 0x1) << 2);
	uint8_t err = send_byte(lcd, code, 0);
	_delay_us(50);
	return err;
}

// Sets CGRAM address
uint8_t LCD_set_CGRAM(LCD_t *lcd, uint8_t addr)
{
	addr &= 0x3F; // CGRAM address takes lower 6 bits
	uint8_t code = 0b01000000 | addr;
	uint8_t err = send_byte(lcd, code, 0);
	_delay_us(50);
	return err;
}

// Sets DDRAM address
uint8_t LCD_set_DDRAM(LCD_t *lcd, uint8_t addr)
{
	addr &= 0x7F; // DDRAM address takes lower 7 bits
	uint8_t code = 0b10000000 | addr;
	uint8_t err = send_byte(lcd, code, 0);
	_delay_us(50);
	return err;
}

// Writes data to LCD
uint8_t LCD_write_data(LCD_t *lcd, uint8_t data)
{;
	uint8_t err = send_byte(lcd, data, (1 << RS));
	lcd->currCol++;
	_delay_us(100);
	return err;
}

// Writes a string to the LCD screen
uint8_t LCD_write_string(LCD_t *lcd, char *str)
{
	char letter = *str;
	while (letter != 0x0)
	{
		uint8_t err = LCD_write_data(lcd, letter);
		if (err != TWI_OK)
		{
			return err;
		}
		letter = *(++str);
	}
	return TWI_OK;
}

// Toggles backlight ON (1) or OFF (0)
uint8_t LCD_toggle_backlight(LCD_t *lcd, uint8_t on)
{
	lcd->isBacklightOn = (on & 0x1);
	return send_twi(lcd, 0); // send_twi sets backlight based on lcd's isBacklightOn
}

// Sets the cursor to the specified row and column
uint8_t LCD_set_cursor(LCD_t *lcd, uint8_t row, uint8_t col)
{
	// Make sure row and col are valid
	if ((lcd->rows <= row) | (lcd->cols <= col)) return -1;
	
	// Set DDRAM
	uint8_t rowOffsets[] = {0x0, 0x40};
	lcd->currRow = row;
	lcd->currCol = col;
	return LCD_set_DDRAM(lcd, rowOffsets[row] + col);
}

// Adds a character defined by charMap (a custom character) to be added to CGRAM at the location specified
uint8_t LCD_add_character(LCD_t *lcd, uint8_t location, uint8_t charMap[])
{
	location &= 0b111; // There are only 7 locations in CGRAM for custom characters
	uint8_t err = LCD_set_CGRAM(lcd, location << 3); // Set CGRAM address
	if (err != TWI_OK) return err;
	for (int i = 0; i < 8; i++) LCD_write_data(lcd, charMap[i]); // Write character map to CGRAM address
	err = LCD_set_cursor(lcd, 0, 0); // Set DDRAM (by setting cursor to (0,0)) to go back to using DDRAM
	return err;
}
