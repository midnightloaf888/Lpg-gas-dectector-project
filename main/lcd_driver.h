#pragma once
#include <stdint.h>

// Sets up the LCD pins and runs the 4-bit initialization sequence.
void lcd_init(void);

// Sends a command byte (RS = 0) — e.g. clear screen, move cursor.
void lcd_command(uint8_t cmd);

// Sends a data byte (RS = 1) — one character.
void lcd_data(uint8_t data);

// Sends a full string, one character at a time.
void lcd_print(const char *str);

// Clears the display and waits the required settle time.
void lcd_clear(void);

// Moves the cursor to (row, col). row is 0 or 1 on a 16x2 display.
void lcd_set_cursor(int row, int col);