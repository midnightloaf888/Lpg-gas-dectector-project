#include "esp_err.h"
#include "driver/gpio.h"

#include "output_control.h"

#define LED_PIN     GPIO_NUM_25
#define BUZZER_PIN  GPIO_NUM_32

// Current test values - replace with calibrated PPM thresholds later
#define WARNING_THRESHOLD  700
#define DANGER_THRESHOLD   1350

void output_control_init(void)
{
    ESP_ERROR_CHECK(gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT));

    gpio_set_level(LED_PIN, 0);
    gpio_set_level(BUZZER_PIN, 0);
}

gas_state_t output_control_evaluate(int adc_raw)
{
    gas_state_t state;

    if (adc_raw < WARNING_THRESHOLD) {
        state.status = "NORMAL";
        state.led_on = false;
        state.buzzer_on = false;
    }
    else if (adc_raw < DANGER_THRESHOLD) {
        state.status = "WARNING";
        state.led_on = true;
        state.buzzer_on = false;
    }
    else {
        state.status = "DANGER";
        state.led_on = true;
        state.buzzer_on = true;
    }

    gpio_set_level(LED_PIN, state.led_on);
    gpio_set_level(BUZZER_PIN, state.buzzer_on);

    return state;
}