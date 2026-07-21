#include <stdio.h>
#include <unistd.h>

#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"

#define ADC_UNIT       ADC_UNIT_1
#define ADC_CHANNEL    ADC_CHANNEL_6   
#define ADC_ATTEN      ADC_ATTEN_DB_12
#define ADC_BITWIDTH   ADC_BITWIDTH_DEFAULT

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

    int adc_raw;

    while (1)
    {
        ESP_ERROR_CHECK(
            adc_oneshot_read(
                adc_handle,
                ADC_CHANNEL,
                &adc_raw
            )
        );

        float voltage = ((float)adc_raw / 4095.0f) * 3.3f;

        printf("ADC Value : %d\n", adc_raw);
        printf("Voltage   : %.2f V\n", voltage);
        printf("-----------------------------\n");

        sleep(1);
    }
}