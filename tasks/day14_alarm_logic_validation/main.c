#include <stdio.h>

#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ADC_UNIT        ADC_UNIT_1
#define ADC_CHANNEL     ADC_CHANNEL_6      
#define ADC_ATTEN       ADC_ATTEN_DB_12
#define ADC_BITWIDTH    ADC_BITWIDTH_DEFAULT

#define LED_PIN         GPIO_NUM_25
#define BUZZER_PIN      GPIO_NUM_26

#define WARNING_THRESHOLD  700
#define DANGER_THRESHOLD   1350

#define SENSOR_DELAY_MS     100
#define DISPLAY_DELAY_MS    2000

typedef enum
{
    STATE_NORMAL,
    STATE_WARNING,
    STATE_DANGER
} system_state_t;

const char *get_state_name(system_state_t state)
{
    switch (state)
    {
        case STATE_NORMAL:
            return "NORMAL";

        case STATE_WARNING:
            return "WARNING";

        case STATE_DANGER:
            return "DANGER";

        default:
            return "UNKNOWN";
    }
}

void set_alarm_outputs(system_state_t state)
{
    switch (state)
    {
        case STATE_NORMAL:
            gpio_set_level(LED_PIN, 0);
            gpio_set_level(BUZZER_PIN, 0);
            break;

        case STATE_WARNING:
            gpio_set_level(LED_PIN, 1);
            gpio_set_level(BUZZER_PIN, 0);
            break;

        case STATE_DANGER:
            gpio_set_level(LED_PIN, 1);
            gpio_set_level(BUZZER_PIN, 1);
            break;
    }
}

void app_main(void)
{
    adc_oneshot_unit_handle_t adc_handle;

    adc_oneshot_unit_init_cfg_t adc_init_config = {
        .unit_id = ADC_UNIT,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(
            &adc_init_config,
            &adc_handle
        )
    );

    adc_oneshot_chan_cfg_t adc_channel_config = {
        .bitwidth = ADC_BITWIDTH,
        .atten = ADC_ATTEN,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc_handle,
            ADC_CHANNEL,
            &adc_channel_config
        )
    );

    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    gpio_reset_pin(BUZZER_PIN);
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);

    gpio_set_level(LED_PIN, 0);
    gpio_set_level(BUZZER_PIN, 0);

    TickType_t last_display_time = xTaskGetTickCount();

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

        system_state_t current_state;

        if (adc_raw < WARNING_THRESHOLD)
        {
            current_state = STATE_NORMAL;
        }
        else if (adc_raw < DANGER_THRESHOLD)
        {
            current_state = STATE_WARNING;
        }
        else
        {
            current_state = STATE_DANGER;
        }

        set_alarm_outputs(current_state);

        TickType_t current_time = xTaskGetTickCount();

        if ((current_time - last_display_time) >=
            pdMS_TO_TICKS(DISPLAY_DELAY_MS))
        {
            printf("\n--------------------------------\n");
            printf("ADC Value : %d\n", adc_raw);
            printf("Voltage   : %.2f V\n", voltage);
            printf("Status    : %s\n", get_state_name(current_state));

            if (current_state == STATE_NORMAL)
            {
                printf("LED       : OFF\n");
                printf("Buzzer    : OFF\n");
            }
            else if (current_state == STATE_WARNING)
            {
                printf("LED       : ON\n");
                printf("Buzzer    : OFF\n");
            }
            else
            {
                printf("LED       : ON\n");
                printf("Buzzer    : ON\n");
            }

            printf("--------------------------------\n");

            last_display_time = current_time;
        }

        vTaskDelay(pdMS_TO_TICKS(SENSOR_DELAY_MS));
    }
}