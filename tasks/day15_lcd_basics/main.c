#include <stdio.h>
#include "driver/gpio.h"
#include "esp_rom_sys.h"

#define LCD_RS  GPIO_NUM_21
#define LCD_EN  GPIO_NUM_22
#define LCD_D4  GPIO_NUM_27
#define LCD_D5  GPIO_NUM_26
#define LCD_D6  GPIO_NUM_18
#define LCD_D7  GPIO_NUM_19

void lcd_delay_ms(int ms) {
    esp_rom_delay_us(ms * 1000);
}

void lcd_pulse_enable() {
    gpio_set_level(LCD_EN, 0);
    esp_rom_delay_us(1);
    gpio_set_level(LCD_EN, 1);
    esp_rom_delay_us(1);
    gpio_set_level(LCD_EN, 0);
    esp_rom_delay_us(100);
}

void lcd_send_nibble(uint8_t nibble) {
    gpio_set_level(LCD_D4, (nibble >> 0) & 0x01);
    gpio_set_level(LCD_D5, (nibble >> 1) & 0x01);
    gpio_set_level(LCD_D6, (nibble >> 2) & 0x01);
    gpio_set_level(LCD_D7, (nibble >> 3) & 0x01);
    lcd_pulse_enable();
}

void lcd_send_byte(uint8_t byte, int is_data) {
    gpio_set_level(LCD_RS, is_data);
    lcd_send_nibble(byte >> 4);
    lcd_send_nibble(byte & 0x0F);
    esp_rom_delay_us(50);
}

void lcd_command(uint8_t cmd) {
    lcd_send_byte(cmd, 0);
}

void lcd_data(uint8_t data) {
    lcd_send_byte(data, 1);
}

void lcd_print(const char *str) {
    while (*str) {
        lcd_data((uint8_t)(*str));
        str++;
    }
}

void lcd_init() {
   
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LCD_RS) |
                        (1ULL << LCD_EN) |
                        (1ULL << LCD_D4) |
                        (1ULL << LCD_D5) |
                        (1ULL << LCD_D6) |
                        (1ULL << LCD_D7),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    gpio_set_level(LCD_RS, 0);
    gpio_set_level(LCD_EN, 0);

    lcd_delay_ms(50);

    lcd_send_nibble(0x03);
    lcd_delay_ms(5);

    lcd_send_nibble(0x03);
    esp_rom_delay_us(150);

    lcd_send_nibble(0x03);
    esp_rom_delay_us(150);

    lcd_send_nibble(0x02);
    esp_rom_delay_us(150);

    lcd_command(0x28);  
    lcd_delay_ms(1);

    lcd_command(0x08);  
    lcd_delay_ms(1);

    lcd_command(0x01);  
    lcd_delay_ms(2);

    lcd_command(0x06);  
    lcd_delay_ms(1);

    lcd_command(0x0C);  
    lcd_delay_ms(1);
}

void app_main(void) {
    lcd_init();

    lcd_command(0x01);
    lcd_delay_ms(2);
    
    lcd_command(0x80);
    lcd_print("Hello Aaryani!");
}