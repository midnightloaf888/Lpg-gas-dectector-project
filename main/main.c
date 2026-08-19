#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "nvs_flash.h"

#include "wifi_handler.h"
#include "mqtt_handler.h"
#include "lcd_driver.h"
#include "gas_sensor.h"
#include "output_control.h"

static void nvs_init(void)
{
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    else {
        ESP_ERROR_CHECK(ret);
    }
}

void app_main(void)
{
    nvs_init();

    gas_sensor_init();
    output_control_init();
    lcd_init();
    wifi_init();

    vTaskDelay(pdMS_TO_TICKS(10000)); // Wait for Wi-Fi and MQTT to connect
    while (1) {
        int adc_raw = gas_sensor_read_raw();
        float voltage = gas_sensor_raw_to_voltage(adc_raw);

        gas_state_t state = output_control_evaluate(adc_raw);

        // ---------------- Serial monitor ----------------
        printf("\n-----------------------------\n");
        printf("ADC Value : %d\n", adc_raw);
        printf("Voltage   : %.2f V\n", voltage);
        printf("Status    : %s\n", state.status);
        printf("LED       : %s\n", state.led_on ? "ON" : "OFF");
        printf("Buzzer    : %s\n", state.buzzer_on ? "ON" : "OFF");

        // ---------------- MQTT ----------------
        mqtt_publish_value(adc_raw);

        printf("-----------------------------\n");

        // ---------------- LCD ----------------
        char line1[17];
        char line2[17];

        snprintf(line1, sizeof(line1), "ADC:%d",adc_raw);
        snprintf(line2, sizeof(line2), "Status:%s", state.status);
        // snprintf(line2, sizeof(line2), "L:%s B:%s",
        //           state.led_on ? "ON" : "OFF",
        //           state.buzzer_on ? "ON" : "OFF");

        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_print(line1);
        lcd_set_cursor(1, 0);
        lcd_print(line2);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}