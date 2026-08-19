#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"

#include "gas_sensor.h"

#define ADC_UNIT     ADC_UNIT_1
#define ADC_CHANNEL  ADC_CHANNEL_6      // GPIO34
#define ADC_ATTEN    ADC_ATTEN_DB_12
#define ADC_BITWIDTH ADC_BITWIDTH_DEFAULT

static adc_oneshot_unit_handle_t adc_handle;

void gas_sensor_init(void)
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    adc_oneshot_chan_cfg_t channel_config = {
        .bitwidth = ADC_BITWIDTH,
        .atten = ADC_ATTEN,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        adc_handle, ADC_CHANNEL, &channel_config));
}

int gas_sensor_read_raw(void)
{
    int adc_raw = 0;
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL, &adc_raw));
    return adc_raw;
}

float gas_sensor_raw_to_voltage(int adc_raw)
{
    return ((float)adc_raw / 4095.0f) * 3.3f;
}