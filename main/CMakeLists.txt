idf_component_register(
    SRCS
        "main.c"
        "wifi_handler.c"
        "mqtt_handler.c"
        "lcd_driver.c"
        "gas_sensor.c"
        "output_control.c"
    INCLUDE_DIRS "."
    REQUIRES
        esp_driver_gpio
        esp_adc
        esp_wifi
        esp_event
        esp_netif
        nvs_flash
        mqtt
)