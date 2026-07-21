#include <stdio.h>

#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ADC_UNIT        ADC_UNIT_1
#define ADC_CHANNEL     ADC_CHANNEL_6      
#define ADC_ATTEN       ADC_ATTEN_DB_12
#define ADC_BITWIDTH    ADC_BITWIDTH_DEFAULT

#define SAMPLE_COUNT    10
#define SAMPLE_DELAY_MS 500             
#define RESULT_DELAY_MS 3000                

#define WARNING_THRESHOLD 800
#define DANGER_THRESHOLD  1500

void app_main(void)
{
    adc_oneshot_unit_handle_t adc_handle;

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(&init_config, &adc_handle)
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

    while (1)
    {
        int adc_sum = 0;

        printf("\n================================\n");
        printf("Collecting 10 ADC readings\n");
        printf("================================\n");

        for (int i = 0; i < SAMPLE_COUNT; i++)
        {
            int adc_raw = 0;

            ESP_ERROR_CHECK(
                adc_oneshot_read(
                    adc_handle,
                    ADC_CHANNEL,
                    &adc_raw
                )
            );

            adc_sum += adc_raw;

            printf("Reading %2d : %d\n", i + 1, adc_raw);

            vTaskDelay(pdMS_TO_TICKS(SAMPLE_DELAY_MS));
        }

        int average_adc = adc_sum / SAMPLE_COUNT;

        float average_voltage =
            ((float)average_adc / 4095.0f) * 3.3f;

        printf("\n--------------------------------\n");
        printf("Average ADC Value : %d\n", average_adc);
        printf("Average Voltage   : %.2f V\n", average_voltage);

        if (average_adc < WARNING_THRESHOLD)
        {
            printf("Status            : NORMAL\n");
        }
        else if (average_adc < DANGER_THRESHOLD)
        {
            printf("Status            : WARNING\n");
        }
        else
        {
            printf("Status            : DANGER\n");
        }

        printf("--------------------------------\n");
        printf("Next set begins in 3 seconds...\n");

        vTaskDelay(pdMS_TO_TICKS(RESULT_DELAY_MS));
    }
}