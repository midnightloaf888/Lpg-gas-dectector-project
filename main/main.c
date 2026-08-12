#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"


// ==============================
// SENSOR CONFIGURATION
// ==============================

#define ADC_UNIT        ADC_UNIT_1
#define ADC_CHANNEL     ADC_CHANNEL_6      // GPIO34
#define ADC_ATTEN       ADC_ATTEN_DB_12
#define ADC_BITWIDTH    ADC_BITWIDTH_DEFAULT


// ==============================
// THRESHOLDS
// ==============================

#define WARNING_THRESHOLD   700
#define DANGER_THRESHOLD    1350


// ==============================
// LED AND BUZZER
// ==============================

#define LED_PIN       GPIO_NUM_25
#define BUZZER_PIN    GPIO_NUM_32


// ==============================
// LCD PINS
// ==============================

#define LCD_RS  GPIO_NUM_21
#define LCD_EN  GPIO_NUM_22

#define LCD_D4  GPIO_NUM_27
#define LCD_D5  GPIO_NUM_26
#define LCD_D6  GPIO_NUM_18
#define LCD_D7  GPIO_NUM_19


// ==============================
// LCD FUNCTIONS
// ==============================

void lcd_delay_ms(int ms)
{
    esp_rom_delay_us(ms * 1000);
}


void lcd_pulse_enable(void)
{
    gpio_set_level(LCD_EN, 0);
    esp_rom_delay_us(1);

    gpio_set_level(LCD_EN, 1);
    esp_rom_delay_us(1);

    gpio_set_level(LCD_EN, 0);
    esp_rom_delay_us(100);
}


void lcd_send_nibble(uint8_t nibble)
{
    gpio_set_level(LCD_D4, (nibble >> 0) & 0x01);
    gpio_set_level(LCD_D5, (nibble >> 1) & 0x01);
    gpio_set_level(LCD_D6, (nibble >> 2) & 0x01);
    gpio_set_level(LCD_D7, (nibble >> 3) & 0x01);

    lcd_pulse_enable();
}


void lcd_send_byte(uint8_t byte, int is_data)
{
    gpio_set_level(LCD_RS, is_data);

    lcd_send_nibble(byte >> 4);
    lcd_send_nibble(byte & 0x0F);

    esp_rom_delay_us(50);
}


void lcd_command(uint8_t cmd)
{
    lcd_send_byte(cmd, 0);
}


void lcd_data(uint8_t data)
{
    lcd_send_byte(data, 1);
}


void lcd_print(const char *str)
{
    while (*str)
    {
        lcd_data((uint8_t)(*str));
        str++;
    }
}


void lcd_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask =
            (1ULL << LCD_RS) |
            (1ULL << LCD_EN) |
            (1ULL << LCD_D4) |
            (1ULL << LCD_D5) |
            (1ULL << LCD_D6) |
            (1ULL << LCD_D7),

        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    gpio_config(&io_conf);

    gpio_set_level(LCD_RS, 0);
    gpio_set_level(LCD_EN, 0);

    lcd_delay_ms(50);

    // 4-bit initialization
    lcd_send_nibble(0x03);
    lcd_delay_ms(5);

    lcd_send_nibble(0x03);
    esp_rom_delay_us(150);

    lcd_send_nibble(0x03);
    esp_rom_delay_us(150);

    lcd_send_nibble(0x02);
    esp_rom_delay_us(150);

    lcd_command(0x28);   // 4-bit, 2-line mode
    lcd_delay_ms(1);

    lcd_command(0x08);   // Display OFF
    lcd_delay_ms(1);

    lcd_command(0x01);   // Clear display
    lcd_delay_ms(2);

    lcd_command(0x06);   // Cursor moves right
    lcd_delay_ms(1);

    lcd_command(0x0C);   // Display ON, cursor OFF
    lcd_delay_ms(1);
}


// ==============================
// MAIN
// ==============================

void app_main(void)
{
    // ------------------------------
    // ADC INITIALIZATION
    // ------------------------------

    adc_oneshot_unit_handle_t adc_handle;

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(
            &init_config,
            &adc_handle
        )
    );

    adc_oneshot_chan_cfg_t channel_config = {
        .bitwidth = ADC_BITWIDTH,
        .atten = ADC_ATTEN,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc_handle,
            ADC_CHANNEL,
            &channel_config
        )
    );


    // ------------------------------
    // LED + BUZZER INITIALIZATION
    // ------------------------------

    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);

    gpio_set_level(LED_PIN, 0);
    gpio_set_level(BUZZER_PIN, 0);


    // ------------------------------
    // LCD INITIALIZATION
    // ------------------------------

    lcd_init();


    // ==============================
    // MAIN LOOP
    // ==============================

    while (1)
    {
        int adc_raw = 0;

        ESP_ERROR_CHECK(
            adc_oneshot_read(
                adc_handle,
                ADC_CHANNEL,
                &adc_raw
            )
        );

        float voltage =
            ((float)adc_raw / 4095.0f) * 3.3f;


        const char *status;

        int led_state = 0;
        int buzzer_state = 0;


        // ------------------------------
        // NORMAL
        // ------------------------------

        if (adc_raw < WARNING_THRESHOLD)
        {
            status = "NORMAL";

            led_state = 0;
            buzzer_state = 0;
        }


        // ------------------------------
        // WARNING
        // ------------------------------

        else if (adc_raw < DANGER_THRESHOLD)
        {
            status = "WARNING";

            led_state = 1;
            buzzer_state = 0;
        }


        // ------------------------------
        // DANGER
        // ------------------------------

        else
        {
            status = "DANGER";

            led_state = 1;
            buzzer_state = 1;
        }


        // Apply outputs

        gpio_set_level(LED_PIN, led_state);
        gpio_set_level(BUZZER_PIN, buzzer_state);


        // ==============================
        // SERIAL MONITOR
        // ==============================

        printf("\n-----------------------------\n");

        printf("ADC Value : %d\n", adc_raw);
        printf("Voltage   : %.2f V\n", voltage);
        printf("Status    : %s\n", status);

        printf(
            "LED       : %s\n",
            led_state ? "ON" : "OFF"
        );

        printf(
            "Buzzer    : %s\n",
            buzzer_state ? "ON" : "OFF"
        );

        printf("-----------------------------\n");


        // ==============================
        // LCD DISPLAY
        // ==============================

        char line1[17];
        char line2[17];

        snprintf(
            line1,
            sizeof(line1),
            "%s ADC:%d",
            status,
            adc_raw
        );

        snprintf(
            line2,
            sizeof(line2),
            "L:%s B:%s",
            led_state ? "ON" : "OFF",
            buzzer_state ? "ON" : "OFF"
        );


        lcd_command(0x01);
        lcd_delay_ms(2);


        // Line 1
        lcd_command(0x80);
        lcd_print(line1);


        // Line 2
        lcd_command(0xC0);
        lcd_print(line2);


        // Update every 2 seconds
        vTaskDelay(
            pdMS_TO_TICKS(2000)
        );
    }
}