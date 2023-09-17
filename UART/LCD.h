
#include <stdint.h>

#ifndef LCD_H_
#define LCD_H_

#define PCF8574_FIXED_ADDR   0b0100
#define PCF8574A_FIXED_ADDR  0b0111

#define RS  0
#define RW  1
#define E   2
#define BACKLIGHT 3
#define DB4 4
#define DB5 5
#define DB6 6
#define DB7 7

typedef struct LCD_t
{
	uint8_t addr;
	uint8_t rows;
	uint8_t cols;
	uint8_t currRow;
	uint8_t currCol;
	uint8_t isBacklightOn;
} LCD_t;

LCD_t LCD_init(uint8_t addr, uint8_t rows, uint8_t cols, uint32_t bus_speed, uint8_t isPCF8574A);

uint8_t LCD_clear_display(LCD_t *lcd);
uint8_t LCD_return_home(LCD_t *lcd);
uint8_t LCD_entry_mode_set(LCD_t *lcd, uint8_t isLtR, uint8_t isShift);
uint8_t LCD_display_toggle(LCD_t *lcd, uint8_t isDisplayOn, uint8_t isCursorOn, uint8_t isCursorBlink);
uint8_t LCD_cursor_display_shift(LCD_t *lcd, uint8_t isDisplayShift, uint8_t isLtR);
uint8_t LCD_function_set(LCD_t *lcd, uint8_t dataLength, uint8_t numLines, uint8_t font);
uint8_t LCD_set_CGRAM(LCD_t *lcd, uint8_t addr);
uint8_t LCD_set_DDRAM(LCD_t *lcd, uint8_t addr);
uint8_t LCD_write_data(LCD_t *lcd, uint8_t data);

uint8_t LCD_write_string(LCD_t *lcd, char *str);
uint8_t LCD_toggle_backlight(LCD_t *lcd, uint8_t on);
uint8_t LCD_set_cursor(LCD_t *lcd, uint8_t row, uint8_t col);
uint8_t LCD_add_character(LCD_t *lcd, uint8_t location, uint8_t charMap[]);

#endif /* LCD_H_ */