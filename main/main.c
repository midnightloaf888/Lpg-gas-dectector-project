#include <stdio.h>
#include <unistd.h>

#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"

#define ADC_UNIT        ADC_UNIT_1
#define ADC_CHANNEL     ADC_CHANNEL_6      // GPIO34
#define ADC_ATTEN       ADC_ATTEN_DB_12
#define ADC_BITWIDTH    ADC_BITWIDTH_DEFAULT

#define CLEAN_AIR_ADC   400
#define LPG_MAX_ADC     1000

void app_main(void)
{
    adc_oneshot_unit_handle_t adc_handle;

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH,
        .atten = ADC_ATTEN,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_CHANNEL, &config));

    int adc_raw;

    while (1)
    {
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL, &adc_raw));

        int gasPercent = ((adc_raw - CLEAN_AIR_ADC) * 100) /
                         (LPG_MAX_ADC - CLEAN_AIR_ADC);

        if (gasPercent < 0)
        {
            gasPercent = 0;
        }

        if (gasPercent > 100)
        {
            gasPercent = 100;
        }

        printf("ADC Value : %d\n", adc_raw);
        printf("Gas Level : %d%%\n", gasPercent);

        if (gasPercent <= 20)
        {
            printf("Status    : SAFE\n");
        }
        else if (gasPercent <= 50)
        {
            printf("Status    : LOW GAS\n");
        }
        else if (gasPercent <= 80)
        {
            printf("Status    : WARNING\n");
        }
        else
        {
            printf("Status    : LPG DETECTED\n");
        }

        printf("-----------------------------\n");

        sleep(2);
    }
}